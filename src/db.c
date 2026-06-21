#include <stdlib.h>
#include <db.h>
#include <db_memory.h>
#include <db_config.h>
#include <db_errors.h>

// BEGIN CLIENT DATA INITIALIZATION
int client_data_init(CLIENT_DATA **client_data)
{
    if (db_alloc((void **)client_data, sizeof(CLIENT_DATA)) == 0) {

        _Static_assert(sizeof(*(*client_data)->repair_requests.array) == sizeof(REPAIR_REQUEST), "Error test2"); // TODO remove

        if (db_alloc((void **)&(*client_data)->repair_requests.array, MIN_REPAIR_REQUEST_INITIAL * sizeof(*(*client_data)->repair_requests.array)) == 0) {
            (*client_data)->repair_requests.array_max_len = MIN_REPAIR_REQUEST_INITIAL;
            return 0;
        }

        db_free((void *)client_data);

        return DB_UNABLE_TO_INIT_REPAIR_REQUEST_IN_CLIENT_DATA;
    }

    return DB_UNABLE_TO_ALLOCATE_CLIENT_USER_MEMORY;
}

void client_data_free(CLIENT_DATA **client_data)
{
    if ((client_data != NULL) && (*client_data != NULL)) {
        // TODO add children destroyers

        if ((*client_data)->repair_requests.array) {
            memset((void *)(*client_data)->repair_requests.array, 0, (*client_data)->repair_requests.array_max_len * sizeof(*((*client_data)->repair_requests.array)));
            free((void *)(*client_data)->repair_requests.array);
            (*client_data)->repair_requests.array = NULL;
        }

        memset((void *)(*client_data), 0, sizeof(CLIENT_DATA));
        free((void *)(*client_data));
        (*client_data) = NULL;
    }
}

// END CLIENT DATA INITIALIZATION

// BEGIN CLEAR ALL EDIT/ADD/DELETE records (NOT STORED IN DATABASE)
int client_data_clear(CLIENT_DATA *client_data)
{
_Static_assert(sizeof(CLIENT_DATA) == sizeof(*client_data), "Error test");
    if (client_data) {
        size_t array_max_len = client_data->repair_requests.array_max_len;
        REPAIR_REQUEST *array = client_data->repair_requests.array;
 
        memset((void *)client_data, 0, sizeof(*client_data));

        client_data->repair_requests.array = array;
        client_data->repair_requests.array_max_len = array_max_len;

        array = NULL;
        array_max_len = 0;
 
        return 0;
    }

    return DB_UNABLE_TO_CLEAR_CLIENT_DATA;
}
// END CLEAR ALL EDIT/ADD/DELETE records (NOT STORED IN DATABASE)

// REPAIR_REQUEST **request and *request ARE NOT NULL
static int repair_request_grow(REPAIR_REQUESTS **requests, size_t plus_n)
{
    if (plus_n > 0) {
        plus_n += (*requests)->array_max_len;

        DB_ALIGN_VEC_LENGTH(plus_n, MIN_REPAIR_REQUEST_INITIAL)

        if (MAX_REPAIR_REQUESTS_LIMIT >= plus_n) {
            REPAIR_REQUEST *new = NULL, *current;

            if (db_alloc(&new, plus_n * sizeof(REPAIR_REQUEST)))
                return DB_UNABLE_TO_GROW_AND_MOVE_REPAIR_REQUESTS;

            current = (*requests)->array;

            memcpy((void *)new, (void *)current, (*requests)->n * sizeof(*new));

            db_clear_and_free((void *)&current, (*requests)->array_max_len * sizeof(*current));

            (*requests)->array = new;
            (*requests)->array_max_len = plus_n;

            new = NULL;

            return 0;
        }
    }

    return DB_UNABLE_TO_GROW_REPAIR_REQUESTS;
}

// BEGIN ADD REPAIR_ARRAY BEFORE SAVE
int client_add_repair_array(CLIENT_DATA *client_data, REPAIR_REQUEST *request, size_t request_len)
{
    if ((client_data != NULL) && (request != NULL) && (request_len > 0)) {
        REPAIR_REQUEST *repair_request_ptr = client_data->repair_requests.array;
        size_t n = client_data->repair_requests.n + request_len;

        if (n > client_data->repair_requests.array_max_len) {
            int err = repair_request_grow(&client_data->repair_requests, request_len);
            if (err == 0)
                client_data->repair_requests.array_max_len = n;
            else
                return err;
        }

        memcpy((void *)&client_data->repair_requests.array[client_data->repair_requests.n], request, request_len * sizeof(*(client_data->repair_requests.array)));
        client_data->repair_requests.n = n;
        return 0;
    }

    return DB_UNABLE_TO_ADD_REPAIR_REQUEST;
}
// END ADD BEFORE SAVE
