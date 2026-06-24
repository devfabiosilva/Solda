#include <stdlib.h>
#include <db.h>
#include <db_memory.h>
#include <db_config.h>
#include <db_errors.h>
#include <db_macros.h>

void service_requests_clear(SERVICE_REQUESTS *service_requests)
{
    DB_NULLABLE_ARRAY_CLEAR(service_requests)
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

void repair_requests_clear(REPAIR_REQUESTS *repair_requests)
{
    if ((repair_requests != NULL) && (repair_requests->array != NULL)) {
        while (repair_requests->n > 1) {
            REPAIR_REQUEST *current_repair_request = &repair_requests->array[--(repair_requests->n)];

            //PUSH ALLOC'd service_request_array size
            size_t array_max_len = current_repair_request->optional_service_requests.array_max_len;
            //PUSH ALLOC'd service_request_array if exists
            SERVICE_REQUEST *service_request_array = current_repair_request->optional_service_requests.array;

            // Clear service_request_array (if exists)
            service_requests_clear(&current_repair_request->optional_service_requests);

            // Clear all elements in current repair request
            DB_CLEAR_NON_NULL_ELEMENT(current_repair_request)

            // POP CLEARED ALLOC'D (IF EXISTS) ARRAY
            current_repair_request->optional_service_requests.array = service_request_array;
            // POP ARRAY CLEARED ARRAY SIZE
            current_repair_request->optional_service_requests.array_max_len = array_max_len;
        }
    }
}

static void repair_requests_free(REPAIR_REQUESTS *repair_requests)
{
    if (repair_requests->array) {

        // For each array, free optional_service_requests arrays (if alloc'd)
        while (repair_requests->n > 1) {
            REPAIR_REQUEST *repair_request_ptr = &repair_requests->array[--(repair_requests->n)];
            SERVICE_REQUESTS *optional_service_requests = &repair_request_ptr->optional_service_requests;
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
/*
static int client_data_init(CLIENT_DATA **client_data)
{
    if (db_alloc((void **)client_data, sizeof(CLIENT_DATA)) == 0) {

        int err = repair_requests_init(&(*client_data)->repair_requests);
        if (err)
            db_free((void **)client_data);

        return err;
    }

    return DB_UNABLE_TO_ALLOCATE_CLIENT_USER_MEMORY;
}
*/
int client_data_request_init(CLIENT_DATA_REQUESTS **client_data_request)
{
    if (db_alloc((void **)&client_data_request, sizeof(*(*client_data_request))) == 0) {

        if (db_alloc((void **)&(*client_data_request)->array, MIN_CLIENT_DATA_REQUEST_INITIAL * sizeof(*(*client_data_request)->array)) == 0) {
            (*client_data_request)->array_max_len = MIN_CLIENT_DATA_REQUEST_INITIAL;
            return 0;
        }

        db_free(client_data_request);

        return DB_UNABLE_TO_INIT_CLIENT_DATA_REQUEST_REQUEST_ARRAY;
    }

    return DB_UNABLE_TO_INIT_CLIENT_DATA_REQUEST_REQUEST;
}

void client_data_request_free(CLIENT_DATA_REQUESTS **client_data_request)
{
    if ((client_data_request != NULL) && (*client_data_request != NULL)) {
        if ((*client_data_request)->array) {

            // For each array, free arrays (if alloc'd)
            while ((*client_data_request)->n > 1) {
                CLIENT_DATA *client_data_ptr = &(*client_data_request)->array[--((*client_data_request)->n)];
                repair_requests_free(client_data_ptr);
                DB_CLEAR_NON_NULL_ELEMENT(client_data_ptr)
                free((void *)client_data_ptr);
            }

            explicit_bzero((void *)(*client_data_request)->array, (*client_data_request)->array_max_len * sizeof(*((*client_data_request)->array)));
            free((void *)(*client_data_request)->array);
            //(*client_data_request)->array = NULL;
        }

        explicit_bzero((void *)(*client_data_request), sizeof(*(*client_data_request)));
        free((void *)(*client_data_request));
        (*client_data_request) = NULL;
    }
}

// END CLIENT DATA INITIALIZATION

// BEGIN CLEAR ALL EDIT/ADD/DELETE records (NOT STORED IN DATABASE)
void client_data_clear(CLIENT_DATA *client_data)
{
_Static_assert(sizeof(CLIENT_DATA) == sizeof(*client_data), "Error test");
    if (client_data) {
        size_t array_max_len = client_data->repair_requests.array_max_len;
        REPAIR_REQUEST *array = client_data->repair_requests.array;
 
        repair_requests_clear(&client_data->repair_requests);
        //explicit_bzero((void *)client_data, sizeof(*client_data));
        DB_CLEAR_NON_NULL_ELEMENT(client_data)

        client_data->repair_requests.array = array;
        client_data->repair_requests.array_max_len = array_max_len;

        array = NULL;
        array_max_len = 0;
    }
}
// END CLEAR ALL EDIT/ADD/DELETE records (NOT STORED IN DATABASE)

// BEGIN NEW REPAIR BEFORE SAVE
// REPAIR_REQUEST **request and *request ARE NOT NULL
GROW_ARRAY_FUNC(repair_request, REPAIR_REQUESTS)
int client_repair_get(size_t *index, REPAIR_REQUEST **request_out, CLIENT_DATA *client_data)
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
// Technician
// END CLIENT MANIPULATION
