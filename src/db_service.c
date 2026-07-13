#include <db.h>
#include <db_service.h>
#include <db_errors.h>
#include <db_memory.h>
#include <string.h>
#include <db_log.h>

int db_service_init(DB_SERVICE **db_service, char *connection)
{
    DB_DEBUG("db_service_init. Initializing instance ...")
    if ((db_service != NULL) && (*db_service == NULL)) {
        if ((connection != NULL) && (connection[0] != 0)) {

_Static_assert(sizeof(*(*db_service)) == sizeof(DB_SERVICE), "Invalid size");

            if (db_alloc((void **)db_service, sizeof(*(*db_service))) == 0) {
                DB_DEBUG("db_service_init. Alloc'd size %zu at %p", sizeof(*(*db_service)), (void *)*db_service)
                if (((*db_service)->conn = PQconnectdb(connection)) == NULL) {
                    DB_DEBUG("db_service_init. Fail init PQconnectdb. Destroying %p instance ...", (void *)(*db_service))
                    _DB_FREE(*db_service);
                    DB_DEBUG("db_service_init. Destroyed")
                    return DB_SERVICE_UNABLE_TO_INITIALIZE_POSTGRES_CLIENT;
                }

                int err = PQstatus((*db_service)->conn);
                if (err != CONNECTION_OK) {
                    fprintf(stderr, "PQstatus error %d with message: %s\n", err, PQerrorMessage((*db_service)->conn));
                    DB_DEBUG("Unable to conect to Postgres service. Destroying Postgres connection and db_service instance")
                    PQfinish((*db_service)->conn);
                    _DB_CLEAR_AND_FREE((*db_service), sizeof(*(*db_service)));
                    DB_DEBUG("Destroyed with error %d", err)
                    return err;
                }

                err = technician_data_requests_init(&((*db_service)->technician_data_requests));
                if (err) {
                    DB_DEBUG("technician_data_requests_init fail @ db_service_init: with error code: %d", err)
                    PQfinish((*db_service)->conn);
                    _DB_CLEAR_AND_FREE((*db_service), sizeof(*(*db_service)));
                    DB_DEBUG("Destroyed db_service_init")
                }
            
                DB_DEBUG("db_service_init: returnig with error code: %d", err)
                return err;
            }

            DB_DEBUG("db_service_init: C memory error. DB_SERVICE_UNABLE_TO_INITIALIZE(%d)", DB_SERVICE_UNABLE_TO_INITIALIZE)
            return DB_SERVICE_UNABLE_TO_INITIALIZE;

        }

        DB_DEBUG("Invalid NULL or EMPTY Postgres connection: DB_SERVICE_UNABLE_TO_INITIALIZE(%d)", DB_SERVICE_UNABLE_TO_INITIALIZE)
        return DB_SERVICE_CONNECTION_ENDPOINT_INVALID;
    }

    DB_DEBUG("db_service_init: C Pointer error. DB_SERVICE_CONNECTION_INVALID_OR_ALREADY_USED(%d)", DB_SERVICE_CONNECTION_INVALID_OR_ALREADY_USED)
    return DB_SERVICE_CONNECTION_INVALID_OR_ALREADY_USED;
}

void db_service_free(DB_SERVICE **db_service)
{
    DB_DEBUG("db_service_free: Prepare to destroy db_service ...")
    if ((db_service != NULL) && (*db_service != NULL)) {
        DB_DEBUG("db_service_free: Call technician_data_requests_free to destroy all technician data requests ...")

        technician_data_requests_free(&(*db_service)->technician_data_requests);

        DB_DEBUG("db_service_free: technician_data_requests destroyed ...")
        DB_DEBUG("db_service_free: Call PQfinish to destroy all Postgres intances ...")

        PQfinish((*db_service)->conn);

        DB_DEBUG("db_service_free: PQfinish connection destroyed...")
        DB_DEBUG("db_service_free: Destroy and zero db_service ...")

        _DB_CLEAR_AND_FREE(*db_service, sizeof(*(*db_service)));

        DB_DEBUG("db_service_free: Destroyed")
    }
}