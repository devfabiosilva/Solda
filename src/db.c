#include <string.h>
#include <stdlib.h>
#include <db.h>
#include <db_memory.h>
#include <db_errors.h>
#include <db_macros.h>
#include <db_log.h>

inline void service_request_clear(SERVICE *service_request)
{
    if (service_request)
        explicit_bzero(service_request, sizeof(*service_request));
}

static void _service_requests_clear(SERVICE_REQUESTS *service_requests)
{
    DB_ARRAY_CLEAR(service_requests)
}

void service_requests_clear(SERVICE_REQUESTS *service_requests)
{
    if (service_requests)
        _service_requests_clear(service_requests);
}

void _repair_request_clear(REPAIR *request)
{
    size_t array_max_len = request->optional_service_requests.array_max_len;
    SERVICE *service_request_array = request->optional_service_requests.array;
    if (service_request_array)
        while (request->optional_service_requests.n > 0) {
            SERVICE *service_request_ptr = &request->optional_service_requests.array[--(request->optional_service_requests.n)];
            //_service_requests_clear(service_request_ptr);
            explicit_bzero(service_request_ptr, sizeof(*service_request_ptr));
        }

    DB_CLEAR_NON_NULL_ELEMENT(request)
    request->optional_service_requests.array = service_request_array;
    request->optional_service_requests.array_max_len = array_max_len;
}

void repair_request_clear(REPAIR *request)
{
    if (request)
        _repair_request_clear(request);
}

static void _repair_requests_clear(REPAIR_REQUESTS *requests)
{
    if (requests->array)
        while (requests->n > 0) {
            REPAIR *repair_request_ptr = &requests->array[--(requests->n)];

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
        while (repair_requests->n > 0) {
            REPAIR *repair_request_ptr = &repair_requests->array[--(repair_requests->n)];
            SERVICE_REQUESTS *optional_service_requests = &repair_request_ptr->optional_service_requests;
            // Last child array: service request
            DB_ARRAY_FREE(optional_service_requests)
        }

        explicit_bzero((void *)repair_requests->array, repair_requests->array_max_len * sizeof(*(repair_requests->array)));
        free((void *)repair_requests->array);
        repair_requests->array = NULL;
    }
}

// BEGIN CLIENT DATA INITIALIZATION

static void client_data_requests_free(CLIENT_DATA_REQUESTS *client_data_request)
{
    if (client_data_request->array) {

        // For each array, free arrays (if alloc'd)
        while (client_data_request->n > 0) {
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
void _client_data_clear(CLIENT_DATA *client_data) {
    size_t array_max_len = client_data->repair_requests.array_max_len;
    REPAIR *array = client_data->repair_requests.array;
 
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

// BEGIN CLIENT MANIPULATION
int technician_data_requests_init(TECHNICIAN_DATA_REQUESTS **requests)
{
    if (db_alloc((void **)requests, sizeof(**requests)) == 0)
        return 0;

    return DB_UNABLE_TO_INIT_TECHNICIAN_DATA_REQUESTS;
}

void technician_data_requests_free(TECHNICIAN_DATA_REQUESTS **requests)
{
    if ((requests != NULL) && (*requests != NULL)) {

        if ((*requests)->array) {
            while ((*requests)->n > 0) {
                TECHNICIAN_DATA *technician_data_ptr = &(*requests)->array[--(*requests)->n];
                client_data_requests_free(&technician_data_ptr->client_requests);
            }

            explicit_bzero((void *)(*requests)->array, (*requests)->array_max_len * sizeof(*((*requests)->array)));
            free((void *)(*requests)->array);
            (*requests)->array = NULL;
        }

        free(*requests);
        *requests = NULL;
    }
}

void _technician_data_clear(TECHNICIAN_DATA *request)
{
    size_t array_max_len = request->client_requests.array_max_len;
    CLIENT_DATA *client_data_array = request->client_requests.array;

    if (client_data_array)
        while (request->client_requests.n > 0) {
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
            while (requests->n > 0) {
                TECHNICIAN_DATA *technician_data_ptr = &requests->array[--(requests->n)];
                _technician_data_clear(technician_data_ptr);
            }

        requests->n = 0;
    }
}

GROW_ARRAY_FUNC(technician_data_requests, TECHNICIAN_DATA)
int technician_acquire_technician_data_from_array(size_t *index, TECHNICIAN_DATA **out, TECHNICIAN_DATA_REQUESTS *in)
{
    int err = 0;
    DB_DEBUG("Entering technician_acquire_technician_data_from_array ...")
    if ((index != NULL) && (out != NULL) && (*out == NULL) && (in != NULL)) {
        *out = NULL;
        *index = 0;

        if (in->array) {
            if (in->n >= in->array_max_len) {
                err = technician_data_requests_grow(in, 1);
                if (err)
                    return err;
            }

            *index = in->n;
            *out = &(in->array[in->n++]);
            DB_DEBUG("technician_acquire_technician_data_from_array: Adding next index %zu in alloc'd array %p", in->n, in->array)

        } else if (db_alloc((void **)&(in->array), MIN_TECHNICIAN_DATA_REQUESTS_INITIAL * sizeof(*(in->array))) == 0) {
            DB_DEBUG("Initialized array in technician_acquire_technician_data_from_array %p of size %zu", (void *)in->array, MIN_TECHNICIAN_DATA_REQUESTS_INITIAL * sizeof(*(in->array)))
            in->array_max_len = MIN_TECHNICIAN_DATA_REQUESTS_INITIAL; 
            *index = 0;
            in->n = 1;
            *out = &in->array[0];
        } else
            err = DB_UNABLE_TO_ACQUIRE_AND_ALLOC_TECHNICIAN_DATA_FROM_ARRAY;

    } else
        err = DB_UNABLE_TO_ACQUIRE_TECHNICIAN_DATA_FROM_ARRAY;

    DB_DEBUG("Exiting technician_acquire_technician_data_from_array with status code %d", err)
    return err;
}

int technician_acquire_client_data_requests_from_array(CLIENT_DATA_REQUESTS **out, size_t technician_index, TECHNICIAN_DATA_REQUESTS *in)
{
    TECHNICIAN_ACQUIRE_CLIENT_DATA_REQUEST_FROM_ARRAY_BEGIN(
        (out != NULL) && (*out == NULL) && (in != NULL),
        *out = NULL;
    )
        *out = &this_technician_data->client_requests;
        return 0;
    TECHNICIAN_ACQUIRE_CLIENT_DATA_REQUEST_FROM_ARRAY_END
}

GROW_ARRAY_FUNC(client_data_requests, CLIENT_DATA)
int technician_acquire_client_data_from_array(size_t *index, CLIENT_DATA **out, size_t technician_index, TECHNICIAN_DATA_REQUESTS *in)
{
    TECHNICIAN_ACQUIRE_CLIENT_DATA_REQUEST_FROM_ARRAY_BEGIN(
        (index != NULL) && (out != NULL) && (*out == NULL) && (in != NULL), 
        *out = NULL;
        *index = 0;
    )
        if (this_technician_data->client_requests.array) {
            if (this_technician_data->client_requests.n >= this_technician_data->client_requests.array_max_len) {
                int err = client_data_requests_grow(&this_technician_data->client_requests, 1);
                if (err)
                    return err;
            }

            *index = this_technician_data->client_requests.n;
            *out = &this_technician_data->client_requests.array[this_technician_data->client_requests.n++];

            return 0;
        }

        if (db_alloc((void **)&(this_technician_data->client_requests.array), MIN_CLIENT_DATA_REQUESTS_INITIAL * sizeof(*(this_technician_data->client_requests.array))) == 0) {
            this_technician_data->client_requests.array_max_len = MIN_CLIENT_DATA_REQUESTS_INITIAL;
            *index = 0;
            this_technician_data->client_requests.n = 1;
            *out = &this_technician_data->client_requests.array[0];
            return 0;
        }

        return DB_UNABLE_TO_ACQUIRE_AND_ALLOC_DATA_REQUESTS_FROM_TECHNICIAN_DATA_REQUESTS;
    TECHNICIAN_ACQUIRE_CLIENT_DATA_REQUEST_FROM_ARRAY_END
}

int technician_acquire_repair_requests_from_array(REPAIR_REQUESTS **out, size_t technician_index, size_t client_data_index, TECHNICIAN_DATA_REQUESTS *in)
{
    TECHNICIAN_ACQUIRE_REPAIR_REQUEST_FROM_ARRAY_BEGIN(
        (out != NULL) && (*out == NULL) && (in != NULL), 
        *out = NULL;
    )
        *out = &this_client_data->repair_requests;

        return 0;
    TECHNICIAN_ACQUIRE_REPAIR_REQUEST_FROM_ARRAY_END
}

GROW_ARRAY_FUNC(repair_request, REPAIR)
int technician_acquire_repair_request_from_array(size_t *index, REPAIR **out, size_t technician_index, size_t client_data_index, TECHNICIAN_DATA_REQUESTS *in)
{
    TECHNICIAN_ACQUIRE_REPAIR_REQUEST_FROM_ARRAY_BEGIN(
        (index != NULL) && (out != NULL) && (*out == NULL) && (in != NULL),
        *out = NULL;
        *index = 0;
    )

        if (this_client_data->repair_requests.array) {
            if (this_client_data->repair_requests.n >= this_client_data->repair_requests.array_max_len) {
                int err = repair_request_grow(&this_client_data->repair_requests, 1);
                if (err)
                    return err;
            }

            *index = this_client_data->repair_requests.n;
            *out = &this_client_data->repair_requests.array[this_client_data->repair_requests.n++];

            return 0;
        }

        if (db_alloc((void **)&(this_client_data->repair_requests.array), MIN_REPAIR_REQUESTS_INITIAL * sizeof(*(this_client_data->repair_requests.array))) == 0) {
            this_client_data->repair_requests.array_max_len = MIN_REPAIR_REQUESTS_INITIAL;
            *index = 0;
            this_client_data->repair_requests.n = 1;
            *out = &this_client_data->repair_requests.array[0];
            return 0;
        }

        return DB_UNABLE_TO_ACQUIRE_AND_ALLOC_REPAIR_REQUEST_FROM_TECHNICIAN_DATA_REQUESTS;

    TECHNICIAN_ACQUIRE_REPAIR_REQUEST_FROM_ARRAY_END
}

int technician_acquire_service_requests_from_array(SERVICE_REQUESTS **out, size_t technician_index, size_t client_data_index, size_t repair_request_index, TECHNICIAN_DATA_REQUESTS *in)
{
    TECHNICIAN_ACQUIRE_SERVICE_REQUEST_FROM_ARRAY_BEGIN(
        (out != NULL) && (*out == NULL) && (in != NULL),
        *out = NULL;
    )
        *out = &this_repair_request->optional_service_requests;
        return 0;
    TECHNICIAN_ACQUIRE_SERVICE_REQUEST_FROM_ARRAY_END
}

GROW_ARRAY_FUNC(service_request, SERVICE)
int technician_acquire_service_request_from_array(size_t *index, SERVICE **out, size_t technician_index, size_t client_data_index, size_t repair_request_index, TECHNICIAN_DATA_REQUESTS *in)
{
    TECHNICIAN_ACQUIRE_SERVICE_REQUEST_FROM_ARRAY_BEGIN(
        (index != NULL) && (out != NULL) && (*out == NULL) && (in != NULL),
        *out = NULL;
        *index = 0;
    )
        if (this_repair_request->optional_service_requests.array) {
            if (this_repair_request->optional_service_requests.n >= this_repair_request->optional_service_requests.array_max_len) {
                int err = service_request_grow(&this_repair_request->optional_service_requests, 1);
                if (err)
                    return err;
            }

            *index = this_repair_request->optional_service_requests.n;
            *out = &this_repair_request->optional_service_requests.array[this_repair_request->optional_service_requests.n++];
            return 0;
        }

        if (db_alloc((void **)&(this_repair_request->optional_service_requests.array), MIN_SERVICE_REQUESTS_INITIAL * sizeof(*(this_repair_request->optional_service_requests.array))) == 0) {
            this_repair_request->optional_service_requests.array_max_len = MIN_SERVICE_REQUESTS_INITIAL;
            *index = 0;
            this_repair_request->optional_service_requests.n = 1;
            *out = &this_repair_request->optional_service_requests.array[0];
            return 0;
        }

        return DB_UNABLE_TO_ACQUIRE_AND_ALLOC_SERVICE_REQUESTS_FROM_TECHNICIAN_DATA_REQUESTS;
    TECHNICIAN_ACQUIRE_SERVICE_REQUEST_FROM_ARRAY_END
}


// END CLIENT MANIPULATION
