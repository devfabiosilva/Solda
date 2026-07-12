#include <db.h>
#include <db_service.h>
#include <db_errors.h>
#include <db_memory.h>
#include <string.h>

int db_service_init(DB_SERVICE **db_service, char *connection)
{
    if ((db_service != NULL) && (*db_service == NULL)) {
        if ((connection != NULL) && (connection[0] != 0)) {

_Static_assert(sizeof(*(*db_service)) == sizeof(DB_SERVICE), "Invalid size");

            if (db_alloc((void **)db_service, sizeof(*(*db_service))) == 0) {

                if (((*db_service)->conn = PQconnectdb(connection)) == NULL) {
                    _DB_FREE(*db_service);
                    
                    return DB_SERVICE_UNABLE_TO_INITIALIZE_POSTGRES_CLIENT;
                }

                int err = PQstatus((*db_service)->conn);
                if (err != CONNECTION_OK) {
                    fprintf(stderr, "PQstatus error %d with message: %s\n", err, PQerrorMessage((*db_service)->conn));
                    PQfinish((*db_service)->conn);
                    _DB_CLEAR_AND_FREE((*db_service), sizeof(*(*db_service)));
                    return err;
                }

                err = technician_data_requests_init(&((*db_service)->technician_data_requests));
                if (err) {
                    PQfinish((*db_service)->conn);
                    _DB_CLEAR_AND_FREE((*db_service), sizeof(*(*db_service)));
                }

                return err;
            }

            return DB_SERVICE_UNABLE_TO_INITIALIZE;

        }

        return DB_SERVICE_CONNECTION_ENDPOINT_INVALID;
    }

    return DB_SERVICE_CONNECTION_INVALID_OR_ALREADY_USED;
}

void db_service_free(DB_SERVICE **db_service)
{
    if ((db_service != NULL) && (*db_service != NULL)) {
        technician_data_requests_free(&(*db_service)->technician_data_requests);
        PQfinish((*db_service)->conn);
        _DB_CLEAR_AND_FREE(*db_service, sizeof(*(*db_service)));
    }
}