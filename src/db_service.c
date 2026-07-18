#include <db.h>
#include <db_service.h>
#include <db_errors.h>
#include <db_memory.h>
#include <string.h>
#include <db_command.h>
#include <db_log.h>
#include <db_messages.h>
#include <arpa/inet.h>
#include <endian.h>

#ifdef SOLDA_DEBUG
    #include <db_time.h>
#endif

const char *QUERY_ALL_TECHNICIANS = "QryAllTechs";

#define MAX_TECHNICIAN_DATA_REQUESTS_LIMIT_BYTES (MAX_TECHNICIAN_DATA_REQUESTS_LIMIT * sizeof(*(*db_service)->technician_data_list))
#define MAX_CLIENT_DATA_REQUESTS_LIMIT_BYTES (MAX_CLIENT_DATA_REQUESTS_LIMIT * sizeof(*(*db_service)->client_data_list))
#define MAX_REPAIR_REQUESTS_LIMIT_BYTES (MAX_REPAIR_REQUESTS_LIMIT * sizeof(*(*db_service)->repair_list))
#define MAX_SERVICE_REQUESTS_LIMIT_BYTES (MAX_SERVICE_REQUESTS_LIMIT * sizeof(*(*db_service)->service_list))

static void _db_service_clear(DB_SERVICE *db_service)
{
    technician_data_requests_clear(db_service->technician_data_requests);

    explicit_bzero((void *)db_service->technician_data_list, sizeof(*(db_service->technician_data_list)));
    db_service->technician_data_list_index = -1;

    explicit_bzero((void *)db_service->client_data_list, sizeof(*(db_service->client_data_list)));
    db_service->client_data_list_index = -1;

    explicit_bzero((void *)db_service->repair_list, sizeof(*(db_service->repair_list)));
    db_service->repair_list_index = -1;

    explicit_bzero((void *)db_service->service_list, sizeof(*(db_service->service_list)));
    db_service->service_list_index = -1;
}

static int _db_add_technician(DB_SERVICE *db_service, TECHNICIAN_DATA *technician_data) {
    if (db_service->technician_data_list_index > -1) {
        if (MAX_TECHNICIAN_DATA_REQUESTS_LIMIT > db_service->technician_data_list_index)
            db_service->technician_data_list[(size_t)(db_service->technician_data_list_index++)] = technician_data;
        else
            return DB_QUERY_ALL_TECHNICIAN_LIST_TECH_OVRFL;
    } else {
        db_service->technician_data_list[0] = technician_data;
        db_service->technician_data_list_index = 1;
    }

    return 0;
}

static int _db_init_query(PGconn *conn)
{
    const Oid param_types[] = { 23, 23 }; 
    DB_DEBUG("Entering _db_init_query ...")
    DB_DEBUG("Preparing query \"%s\" ...", QUERY_ALL_TECHNICIANS)
    PGresult *res = PQprepare(
        conn, QUERY_ALL_TECHNICIANS,
        "select id, name, created_at, email, rules, version, phone_number from technician_data "
        "order by name "
        "limit $1 offset $2",
        2, param_types
    );

    DB_DEBUG("Check query \"%s\" ...", QUERY_ALL_TECHNICIANS)
    int err = 0;
    if (res) {
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            DB_ERROR("_db_init_query: PQresultStatus returned error for query QUERY_ALL_TECHNICIANS with message: %s", PQresultErrorMessage(res))
            err = DB_QUERY_ALL_TECHNICIANS_FAILED;
        }
        PQclear(res);
    } else
        err = DB_SERVICE_PREPARE_QUERY_ALL_TECHNICIAN_DATA_OUT_OF_MEMORY;

    DB_DEBUG("_db_init_query status: %d ...", err)
    return err;
}

int db_service_init(DB_SERVICE **db_service, char *connection)
{
    DB_DEBUG("db_service_init. Initializing instance ...")
    if ((db_service != NULL) && (*db_service == NULL)) {
        if ((connection != NULL) && (connection[0] != 0)) {

_Static_assert(sizeof(*(*db_service)) == sizeof(DB_SERVICE), "Invalid size");

            if (db_alloc((void **)db_service, sizeof(*(*db_service))) == 0) {
                DB_DEBUG("db_service_init. Alloc'd size %zu at %p", sizeof(*(*db_service)), (void *)*db_service)
                int err = 0;
                DB_DEBUG("Initializing Postgres connection ...")
                if (((*db_service)->conn = PQconnectdb(connection)) == NULL) {
                    DB_ERROR("Unable to start to connect to Postgres service. Aborting ...");
                    DB_DEBUG("Unable to start to connect to Postgres service. Destroying Postgres connection and db_service instance")
                    err = DB_SERVICE_UNABLE_TO_INITIALIZE_POSTGRES_CLIENT;
                    goto db_service_init_exit1;
                }
                DB_DEBUG("Postgres connection SUCCESS ...")

                DB_DEBUG("Check Postgres status ...")
                err = PQstatus((*db_service)->conn);
                if (err != CONNECTION_OK) {
                    DB_ERROR("PQstatus error %d with message: %s\n", err, PQerrorMessage((*db_service)->conn));
                    DB_DEBUG("Unable to conect to Postgres service. Destroying Postgres connection and db_service instance")
                    goto db_service_init_exit2;
                }
                DB_DEBUG("Check Postgres status SUCCESS ...")

                DB_DEBUG("db_service_init. Begin allocation (*db_service)->technician_data_list ...")
                if (db_alloc((void **)&(*db_service)->technician_data_list, MAX_TECHNICIAN_DATA_REQUESTS_LIMIT_BYTES)) {
                    err = DB_SEVICE_UNABLE_TO_INIT_TECHNICIAN_DATA_RESQUEST_LIST;
                    DB_ERROR("Unable to allocate (*db_service)->technician_data_list. Aborting ...")
                    goto db_service_init_exit2;
                }
                (*db_service)->technician_data_list_index = -1;
                DB_DEBUG("db_service_init. allocation (*db_service)->technician_data_list %p of size %zu bytes SUCCESS", (*db_service)->technician_data_list, MAX_TECHNICIAN_DATA_REQUESTS_LIMIT_BYTES)

                DB_DEBUG("db_service_init. Begin allocation (*db_service)->client_data_list ...")
                if (db_alloc((void **)&(*db_service)->client_data_list, MAX_CLIENT_DATA_REQUESTS_LIMIT_BYTES)) {
                    err = DB_SEVICE_UNABLE_TO_INIT_CLIENT_DATA_LIST;
                    DB_ERROR("Unable to allocate (*db_service)->client_data_list. Aborting ...")
                    goto db_service_init_exit3;
                }
                (*db_service)->client_data_list_index = -1;
                DB_DEBUG("db_service_init. allocation (*db_service)->client_data_list %p of size %zu bytes SUCCESS", (*db_service)->client_data_list, MAX_CLIENT_DATA_REQUESTS_LIMIT_BYTES)

                DB_DEBUG("db_service_init. Begin allocation (*db_service)->repair_list ...")
                if (db_alloc((void **)&(*db_service)->repair_list, MAX_REPAIR_REQUESTS_LIMIT_BYTES)) {
                    err = DB_SEVICE_UNABLE_TO_INIT_REPAIR_LIST;
                    DB_ERROR("Unable to allocate (*db_service)->repair_list. Aborting ...")
                    goto db_service_init_exit4;
                }
                (*db_service)->repair_list_index = -1;
                DB_DEBUG("db_service_init. allocation (*db_service)->repair_list %p of size %zu bytes SUCCESS", (*db_service)->repair_list, MAX_REPAIR_REQUESTS_LIMIT_BYTES)

                DB_DEBUG("db_service_init. Begin allocation (*db_service)->service_list ...")
                if (db_alloc((void **)&(*db_service)->service_list, MAX_SERVICE_REQUESTS_LIMIT_BYTES)) {
                    err = DB_SEVICE_UNABLE_TO_INIT_SERVICE_LIST;
                    DB_ERROR("Unable to allocate (*db_service)->service_list. Aborting ...")
                    goto db_service_init_exit5;
                }
                (*db_service)->service_list_index = -1;
                DB_DEBUG("db_service_init. allocation (*db_service)->service_list %p of size %zu bytes SUCCESS", (*db_service)->service_list, MAX_SERVICE_REQUESTS_LIMIT_BYTES)

                err = _db_init_query((*db_service)->conn);

                if (err) {
                    DB_ERROR("db_service_init: Load queries failed with error %d", err)
                    goto db_service_init_exit6;
                }

                err = technician_data_requests_init(&((*db_service)->technician_data_requests));
                if (err) {
                    DB_DEBUG("technician_data_requests_init fail @ db_service_init: with error code: %d", err)
                    goto db_service_init_exit6;
                }
            
                DB_DEBUG("db_service_init: returnig SUCCESS")
                return 0;

db_service_init_exit6:
                DB_DEBUG("Destroying (*db_service)->service_list %p ...", (*db_service)->service_list)
                _DB_FREE((*db_service)->service_list);
                DB_DEBUG("Destroyed (*db_service)->service_list")

db_service_init_exit5:
                DB_DEBUG("Destroying (*db_service)->repair_list %p ...", (*db_service)->repair_list)
                _DB_FREE((*db_service)->repair_list);
                DB_DEBUG("Destroyed (*db_service)->repair_list")

db_service_init_exit4:
                DB_DEBUG("Destroying (*db_service)->client_data_list %p ...", (*db_service)->client_data_list)
                _DB_FREE((*db_service)->client_data_list);
                DB_DEBUG("Destroyed (*db_service)->client_data_list")

db_service_init_exit3:
                DB_DEBUG("Destroying (*db_service)->technician_data_list %p ...", (*db_service)->technician_data_list)
                _DB_FREE((*db_service)->technician_data_list);
                DB_DEBUG("Destroyed (*db_service)->technician_data_list")

db_service_init_exit2:
                DB_DEBUG("Destroying (*db_service)->conn %p ...", (*db_service)->conn)
                PQfinish((*db_service)->conn);
                DB_DEBUG("Destroyed (*db_service)->conn")

db_service_init_exit1:
                DB_DEBUG("Destroying (*db_service) %p ...", (*db_service))
                _DB_CLEAR_AND_FREE((*db_service), sizeof(*(*db_service)))
                DB_DEBUG("db_service_init. Destroyed with error %d", err)
                return err;
            }

            DB_DEBUG("db_service_init: C memory error. DB_SERVICE_UNABLE_TO_INITIALIZE(%d)", DB_SERVICE_UNABLE_TO_INITIALIZE)
            return DB_SERVICE_UNABLE_TO_INITIALIZE;

        }

        *db_service = NULL;
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

        DB_DEBUG("db_service_free: technician_data_requests destroying %p ...", (*db_service)->technician_data_requests)
        technician_data_requests_free(&(*db_service)->technician_data_requests);
        DB_DEBUG("db_service_free: technician_data_requests destroyed %p ...", (*db_service)->technician_data_requests)

        DB_DEBUG("db_service_free: Destroy and zero service_list %p of size %zu bytes ...", (*db_service)->service_list, MAX_SERVICE_REQUESTS_LIMIT_BYTES)
        _DB_CLEAR_AND_FREE((*db_service)->service_list, MAX_SERVICE_REQUESTS_LIMIT_BYTES)
        DB_DEBUG("db_service_free: Destroyed service_list %p", (*db_service)->service_list)

        DB_DEBUG("db_service_free: Destroy and zero repair_list %p of size %zu bytes ...", (*db_service)->repair_list, MAX_REPAIR_REQUESTS_LIMIT_BYTES)
        _DB_CLEAR_AND_FREE((*db_service)->repair_list, MAX_REPAIR_REQUESTS_LIMIT_BYTES)
        DB_DEBUG("db_service_free: Destroyed repair_list %p", (*db_service)->repair_list)

        DB_DEBUG("db_service_free: Destroy and zero client_data_list %p of size %zu bytes ...", (*db_service)->client_data_list, MAX_CLIENT_DATA_REQUESTS_LIMIT_BYTES)
        _DB_CLEAR_AND_FREE((*db_service)->client_data_list, MAX_CLIENT_DATA_REQUESTS_LIMIT_BYTES)
        DB_DEBUG("db_service_free: Destroyed client_data_list %p", (*db_service)->client_data_list)

        DB_DEBUG("db_service_free: Destroy and zero technician_data_list %p of size %zu bytes ...", (*db_service)->technician_data_list, MAX_TECHNICIAN_DATA_REQUESTS_LIMIT_BYTES)
        _DB_CLEAR_AND_FREE((*db_service)->technician_data_list, MAX_TECHNICIAN_DATA_REQUESTS_LIMIT_BYTES)
        DB_DEBUG("db_service_free: Destroyed technician_data_list %p", (*db_service)->technician_data_list)

        DB_DEBUG("db_service_free: Call PQfinish to destroy all Postgres instances %p ...", (*db_service)->conn)

        PQfinish((*db_service)->conn);

        DB_DEBUG("db_service_free: PQfinish connection destroyed...")
        DB_DEBUG("db_service_free: Destroy and zero db_service %p of size %zu bytes ...", (*db_service), sizeof(*(*db_service)))

        _DB_CLEAR_AND_FREE(*db_service, sizeof(*(*db_service)));

        DB_DEBUG("db_service_free: Destroyed %p", *db_service)
    }
}

#define DB_SERVICE_CHECK_CONN(error, message_prefix) \
if (PQstatus(db_service->conn) != CONNECTION_OK) { \
    set_db_service_error( \
        db_service, \
        error, \
        message_prefix ". PostgreSQL connection is not active or is closed." \
    ); \
    DB_SERVICE_RETURN \
}

time_t _get_pg_time(PGresult *res, int row, int col)
{
    int64_t ret;
    if (!PQgetisnull(res, row, col)) {
        memcpy(&ret, PQgetvalue(res, row, col), sizeof(ret));
        ret = be64toh(ret); // BE to LE
        ret /= 1000000LL; // Convert microsseconds to seconds ...
        ret += 946684800LL; // Add 30 Years in seconds (postgres initial timestamp 2000)
        if (ret > 0)
            return ret;
    }

    return 0;
}

int32_t _get_pg_i32(PGresult *res, int row, int col)
{
    int32_t ret;
    if (!PQgetisnull(res, row, col)) {
        memcpy(&ret, PQgetvalue(res, row, col), sizeof(ret));
        return (int32_t)ntohl(ret);
    }

    return -1;
}

int db_service_load_technicians(DB_SERVICE *db_service, uint32_t limit, uint32_t offset)
{
    DB_DEBUG("Entering db_service_load_technicians ...")
    if (db_service) {

        uint32_t limit_be  = htonl((uint32_t)limit);
        uint32_t offset_be = htonl((uint32_t)offset);

        const char *param_values[] = { (const char *)&limit_be, (const char *)&offset_be };
        int param_lengths[]        = { (int)sizeof(limit_be), (int)sizeof(offset_be) };
        int param_formats[]        = { 1, 1 };

        DB_DEBUG("db_service_load_technicians: Check PostgreSQL connection ...")
        DB_SERVICE_CHECK_CONN(
            DB_SERVICE_CLOSED_OR_OFFLINE_OR_NOT_AVAILABLE,
            "Load technicians query failed"
        )

        DB_DEBUG("db_service_load_technicians: Execute query ...")
        PGresult *res = PQexecPrepared(
            db_service->conn,
            QUERY_ALL_TECHNICIANS, 
            2,
            param_values,
            param_lengths,
            param_formats,
            1 // 0 = string value as result | 1 = for binary
        );
        DB_DEBUG("db_service_load_technicians: Check results ...")
        if (res) {
            if (PQresultStatus(res) == PGRES_TUPLES_OK) {
                // SUCCESS
                DB_DEBUG("Cleaning all last requests ...")
                _db_service_clear(db_service);

                DB_DEBUG("Begin copy to allocated registries")

                int err;
                size_t index;
                TECHNICIAN_DATA *out;
                int rows = PQntuples(res);

                for (int i = 0; i < rows; i++) {
                    out = NULL;
                    if ((err = technician_acquire_technician_data_from_array(&index, &out, db_service->technician_data_requests)) == 0) {
                        //id, name, created_at, email, rules, version,
                        //phone_number from technician_data
                        
                        int32_t id = _get_pg_i32(res, i, 0);
                        const char *name = PQgetvalue(res, i, 1);
                        time_t created_at = _get_pg_time(res, i, 2);
                        const char *email = PQgetvalue(res, i, 3);
                        int32_t rules = _get_pg_i32(res, i, 4);
                        int32_t version = _get_pg_i32(res, i, 5);
                        const char *phone_number = PQgetvalue(res, i, 6);
#ifdef SOLDA_DEBUG
                        char buffer[64];
                        DB_DEBUG("Created at: %s", db_time(&buffer[0], sizeof(buffer), &created_at))
#endif
                        DB_DEBUG("Technician ID: %u", id)
                        DB_DEBUG("Technician name: %s", name)
                        DB_DEBUG("Technician email: %s", email)
                        DB_DEBUG("Technician rules: %d", rules)
                        DB_DEBUG("Technician version: %d", version)
                        if ((err = _db_add_technician(db_service, out)) == 0) {
                            err = TECHNICIAN_EXECUTE_ADD(
                                out, 
                                TECHNICIAN_ADD_ID(id),
                                TECHNICIAN_ADD_NAME(name),
                                TECHNICIAN_ADD_CREATED_AT(created_at),
                                TECHNICIAN_ADD_EMAIL(email),
                                TECHNICIAN_ADD_RULES(rules),
                                TECHNICIAN_ADD_VERSION(version)
                            )
                            out->flag = TECHNICIAN_READ_FROM_DATABASE;
                            if (err) {
                                set_db_service_error(
                                    db_service,
                                    err,
                                    "Fail TECHNICIAN_EXECUTE_ADD @ db_service_load_technicians: at index %d. Unable to add to temporary list",
                                    i
                                );
                                _db_service_clear(db_service);
                                break;
                            }
                        } else {
                            set_db_service_error(
                                db_service,
                                err,
                                "Fail _db_add_technician @ db_service_load_technicians: at index %d. Unable to add to temporary list",
                                i
                            );
                            _db_service_clear(db_service);
                            break;
                        }
                    } else {
                        set_db_service_error(
                            db_service,
                            err,
                            "Fail technician_acquire_technician_data_from_array @ db_service_load_technicians: at index %d",
                            i
                        );
                        _db_service_clear(db_service);
                        break;
                    }
                }
                DB_DEBUG("Ending copying to allocated registries")
            } else {
                const char *sqlstate = PQresultErrorField(res, PG_DIAG_SQLSTATE);
                set_db_service_error(
                    db_service,
                    DB_QUERY_ALL_TECHNICIANS_EXECUTION_QUERY_ERROR,
                    "Execute load all technician query failed. SQL state: %s. Postgres message %s",
                    ((sqlstate)?sqlstate:"None"),
                    PQresultErrorMessage(res)
                );
            }

            PQclear(res);
        } else
            set_db_service_error(
                db_service,
                DB_QUERY_ALL_TECHNICIANS_EXECUTION_OUT_OF_MEMORY,
                "Error. Execute load all technician query failed. Out of memory"
            );

        DB_DEBUG("db_service_load_technicians: Query result status: %d", db_service->err)
        DB_SERVICE_RETURN
    }

    set_db_service_error(
        db_service,
        DB_SERVICE_UNABLE_TO_RETRIEVE_TECHNICIANS_REQUESTS,
        "Null pointer for db_service. Unable to execute"
    );

    DB_DEBUG("db_service_load_technicians: Invalid pointer")
    DB_SERVICE_RETURN
}
