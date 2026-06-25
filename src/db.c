#include <stdlib.h>
#include <db.h>
#include <db_memory.h>
#include <db_config.h>
#include <db_errors.h>
#include <db_macros.h>

static void _service_requests_clear(SERVICE_REQUESTS *service_requests)
{
    DB_ARRAY_CLEAR(service_requests)
}

void service_requests_clear(SERVICE_REQUESTS *service_requests)
{
    if (service_requests)
        _service_requests_clear(service_requests);
}

static int repair_requests_init(REPAIR_REQUESTS *repair_requests)
{
    _Static_assert(sizeof(*(repair_requests)->array) == sizeof(REPAIR_REQUEST), "Error test2"); // TODO remove

    if (db_alloc((void **)&repair_requests->array, MIN_REPAIR_REQUESTS_INITIAL * sizeof(*(repair_requests)->array)) == 0) {
        //SERVICE_REQUESTS optional_service_requests; is not initialized because it is optional
        repair_requests->array_max_len = MIN_REPAIR_REQUESTS_INITIAL;
        return 0;
    }

    return DB_UNABLE_TO_INIT_REPAIR_REQUEST_IN_CLIENT_DATA;
}

void _repair_request_clear(REPAIR_REQUEST *request)
{
    size_t array_max_len = request->optional_service_requests.array_max_len;
    SERVICE_REQUEST *service_request_array = request->optional_service_requests.array;
    if (service_request_array)
        while (request->optional_service_requests.n > 1) {
            SERVICE_REQUEST *service_request_ptr = &request->optional_service_requests.array[--(request->optional_service_requests.n)];
            _service_requests_clear(service_request_ptr);
        }

    DB_CLEAR_NON_NULL_ELEMENT(request)
    request->optional_service_requests.array = service_request_array;
    request->optional_service_requests.array_max_len = array_max_len;
}

void repair_request_clear(REPAIR_REQUEST *request)
{
    if (request)
        _repair_request_clear(request);
}

static void _repair_requests_clear(REPAIR_REQUESTS *requests)
{
    if (requests->array)
        while (requests->n > 1) {
            REPAIR_REQUEST *repair_request_ptr = &requests->array[--(requests->n)];

            _repair_request_clear(repair_request_ptr);
        }

    requests->n = 0;
}

void repair_requests_clear(REPAIR_REQUESTS *repair_requests)
{
    if (repair_requests)
        _repair_requests_clear(repair_requests);

}

static void repair_requests_free(REPAIR_REQUESTS *repair_requests)
{
    if (repair_requests->array) {

        // For each array, free optional_service_requests arrays (if alloc'd)
        while (repair_requests->n > 1) {
            REPAIR_REQUEST *repair_request_ptr = &repair_requests->array[--(repair_requests->n)];
            SERVICE_REQUESTS *optional_service_requests = &repair_request_ptr->optional_service_requests;
            // Last child array: service request
            DB_ARRAY_FREE(optional_service_requests)
        }

        explicit_bzero((void *)repair_requests->array, repair_requests->array_max_len * sizeof(*(repair_requests->array)));
        free((void *)repair_requests->array);
        repair_requests->array = NULL;
    }
}

GROW_ARRAY_FUNC(service_request, SERVICE_REQUESTS)
int repair_new_service_request(size_t *service_request_index, SERVICE_REQUEST **service_request_out, REPAIR_REQUEST *repair_request)
{
    *service_request_out = NULL;
    if (repair_request) {
        if (repair_request->optional_service_requests.array) {
            if (repair_request->optional_service_requests.n >= repair_request->optional_service_requests.array_max_len) {
                int err = service_request_grow(&repair_request->optional_service_requests, 1);
                if (err)
                    return err;
            }
        } else if (db_alloc((void **)&repair_request->optional_service_requests.array, MIN_SERVICE_REQUESTS_INITIAL * sizeof(*(&repair_request->optional_service_requests)->array)) == 0) {
            // Optional. If new at first time, create array element
            repair_request->optional_service_requests.array_max_len = MIN_SERVICE_REQUESTS_INITIAL;
        } else
            return DB_UNABLE_TO_INIT_SERVICE_REQUESTS_IN_REPAIR_REQUEST;

        if (service_request_index)
            *service_request_index = repair_request->optional_service_requests.n;
    
        *service_request_out = &repair_request->optional_service_requests.array[repair_request->optional_service_requests.n++];

        return 0;
    }

    return DB_UNABLE_TO_ADD_SERVICE_REQUEST;
}

// BEGIN CLIENT DATA INITIALIZATION

static int client_data_requests_init(CLIENT_DATA_REQUESTS *client_data_request)
{
    if (db_alloc((void **)&(client_data_request)->array, MIN_CLIENT_DATA_REQUESTS_INITIAL * sizeof(*(client_data_request)->array)) == 0) {
        (client_data_request)->array_max_len = MIN_CLIENT_DATA_REQUESTS_INITIAL;
        return 0;
    }

    return DB_UNABLE_TO_INIT_CLIENT_DATA_REQUEST_REQUEST_ARRAY;
}

static void client_data_requests_free(CLIENT_DATA_REQUESTS *client_data_request)
{
    if ((client_data_request)->array) {

        // For each array, free arrays (if alloc'd)
        while (client_data_request->n > 1) {
            CLIENT_DATA *client_data_ptr = &client_data_request->array[--(client_data_request)->n];
            repair_requests_free(&client_data_ptr->repair_requests);
        }

        explicit_bzero((void *)client_data_request->array, client_data_request->array_max_len * sizeof(*(client_data_request->array)));
        free((void *)client_data_request->array);
        client_data_request->array = NULL;
    }
}

// END CLIENT DATA INITIALIZATION

// BEGIN CLEAR ALL EDIT/ADD/DELETE records (NOT STORED IN DATABASE)
static void _client_data_clear(CLIENT_DATA *client_data) {
    size_t array_max_len = client_data->repair_requests.array_max_len;
    REPAIR_REQUEST *array = client_data->repair_requests.array;
 
    _repair_requests_clear(&client_data->repair_requests);
    DB_CLEAR_NON_NULL_ELEMENT(client_data)

    client_data->repair_requests.array = array;
    client_data->repair_requests.array_max_len = array_max_len;
}

void client_data_clear(CLIENT_DATA *client_data)
{
_Static_assert(sizeof(CLIENT_DATA) == sizeof(*client_data), "Error test");
    if (client_data) 
        _client_data_clear(client_data);
}

// END CLEAR ALL EDIT/ADD/DELETE records (NOT STORED IN DATABASE)

// BEGIN NEW REPAIR BEFORE SAVE
// REPAIR_REQUEST **request and *request ARE NOT NULL
GROW_ARRAY_FUNC(repair_request, REPAIR_REQUESTS)
int client_aquire_repair(size_t *index, REPAIR_REQUEST **request_out, CLIENT_DATA *client_data)
{
    *request_out = NULL;
    if (client_data != NULL) {

        if (client_data->repair_requests.n >= client_data->repair_requests.array_max_len) {
            int err = repair_request_grow(&client_data->repair_requests, 1);
            if (err)
                return err;
        }

        if (index)
            *index = client_data->repair_requests.n;

        *request_out = &(client_data->repair_requests.array[client_data->repair_requests.n++]);

        return 0;
    }

    return DB_UNABLE_TO_ADD_REPAIR_REQUEST;
}
// END NEW REPAIR BEFORE SAVE

// BEGIN CLIENT MANIPULATION
int technician_data_requests_init(TECHNICIAN_DATA_REQUESTS **requests)
{
    if (db_alloc((void **)requests, sizeof(*requests)) == 0)
        return 0;

    return DB_UNABLE_TO_INIT_TECHNICIAN_DATA_REQUESTS;
}

GROW_ARRAY_FUNC(client_data_requests, CLIENT_DATA_REQUESTS)
int technician_acquire_client_data(size_t *index, CLIENT_DATA **out, TECHNICIAN_DATA *in)
{
    int err = 0;
    if ((index != NULL) && (out != NULL) && (in != NULL)) {
        *index = 0;
        *out = NULL;

        if (in->client_requests.array) {
            if (in->client_requests.n >= in->client_requests.array_max_len) {
                err = client_data_requests_grow(&in->client_requests, 1);
                if (err)
                    return err;
            }

            *index = in->client_requests.n;
            *out = &(in->client_requests.array[in->client_requests.n++]);

        } else if (db_alloc((void **)&(in->client_requests.array), MIN_CLIENT_DATA_REQUESTS_INITIAL * sizeof(*(in->client_requests.array))) == 0) {
            in->client_requests.array_max_len = MIN_CLIENT_DATA_REQUESTS_INITIAL; 
            *index = 0;
            in->client_requests.n = 1;
            *out = &in->client_requests.array[0];
        } else 
            err = DB_UNABLE_TO_ALLOC_ACQUIRE_CLIENT_DATA;
    } else
        err = DB_UNABLE_TO_ACQUIRE_CLIENT_DATA;

    return err;
}

void technician_data_requests_free(TECHNICIAN_DATA_REQUESTS **requests)
{
    if ((requests != NULL) && (*requests != NULL)) {

        if ((*requests)->array)
            while ((*requests)->n > 1) {
                TECHNICIAN_DATA *technician_data_ptr = &(*requests)->array[--(*requests)->n];
                client_data_requests_free(&technician_data_ptr->client_requests);
            }

        explicit_bzero((void *)(*requests)->array, (*requests)->array_max_len * sizeof(*((*requests)->array)));
        free((void *)(*requests)->array);
        (*requests)->array = NULL;
        free(*requests);
        *requests = NULL;
    }
}

static void _technician_data_clear(TECHNICIAN_DATA *request)
{
    size_t array_max_len = request->client_requests.array_max_len;
    CLIENT_DATA *client_data_array = request->client_requests.array;

    if (client_data_array)
        while (request->client_requests.n > 1) {
            CLIENT_DATA *client_data_ptr = &request->client_requests.array[--(request->client_requests.n)];
            _client_data_clear(client_data_ptr);
        }

    DB_CLEAR_NON_NULL_ELEMENT(request)
    request->client_requests.array = client_data_array;
    request->client_requests.array_max_len = array_max_len;
}

void technician_data_clear(TECHNICIAN_DATA *request)
{
    if (request)
        _technician_data_clear(request);
}

void technician_data_requests_clear(TECHNICIAN_DATA_REQUESTS *requests)
{
    if (requests) {
        if (requests->array)
            while (requests->n > 1) {
                TECHNICIAN_DATA *technician_data_ptr = &requests->array[--(requests->n)];
                _technician_data_clear(technician_data_ptr);
            }

        requests->n = 0;
    }
}

GROW_ARRAY_FUNC(technician_data_requests, TECHNICIAN_DATA_REQUESTS)
int technician_acquire_technician_data_from_array(size_t *index, TECHNICIAN_DATA **out, TECHNICIAN_DATA_REQUESTS *in)
{
    if ((index != NULL) && (out != NULL) && (in != NULL)) {
        *out = NULL;
        *index = 0;

        int err = 0;
        if (in->array) {
            if (in->n >= in->array_max_len) {
                err = technician_data_requests(&in, 1);
                if (err)
                    return err;
            }

            *index = in->n;
            *out = &(in->array[in->n++]);

        } else if (db_alloc((void **)&(in->array), MIN_TECHNICIAN_DATA_REQUESTS_INITIAL * sizeof(*(in->array))) == 0) {
            in->array_max_len = MIN_TECHNICIAN_DATA_REQUESTS_INITIAL; 
            *index = 0;
            in->n = 1;
            *out = &in->array[0];
        } else
            err = DB_UNABLE_TO_ACQUIRE_AND_ALLOC_TECHNICIAN_DATA_FROM_ARRAY;

        return err;
    }

    return DB_UNABLE_TO_ACQUIRE_TECHNICIAN_DATA_FROM_ARRAY;
}
// END CLIENT MANIPULATION
