#include <db.h>
#include <db_service.h>
#include <db_errors.h>
#include <db_memory.h>
#include <string.h>
#include <db_command.h>
#include <db_log.h>
#include <db_messages.h>
#include <arpa/inet.h>
#include <db_service_macros.h>
#include <endian.h>
#include <catalog/pg_type_d.h>
#include <db_type.h>
//#include <postgresql/14/server/catalog/pg_type_d.h>

#ifdef SOLDA_DEBUG
    #include <db_time.h>
#endif

extern int _db_alloc_align(void **, size_t);

#define DB_EMPTY_JSON_ARRAY "[]"
#define DB_EMPTY_JSON_ARRAY_LEN (sizeof(DB_EMPTY_JSON_ARRAY) - 1)
const char *QUERY_ALL_TECHNICIANS = "QryAllTechs";
const char *QUERY_ALL_TECHNICIANS_JSON = "QryAllTechsJson";
const char *QUERY_ALL_CLIENTS = "QryAllClients";
const char *QUERY_ALL_CLIENTS_JSON = "QryAllClientsJson";
const char *QUERY_ALL_REPAIR_REQUEST = "QryAllRepairReq";
const char *QUERY_ALL_REPAIR_REQUEST_JSON = "QryAllRepairReqJson";
const char *QUERY_ALL_SERVICE_REQUEST = "QryAllServiceReq";
const char *QUERY_ALL_SERVICE_REQUEST_JSON = "QryAllServiceReqJson";

#define MAX_TECHNICIAN_DATA_REQUESTS_LIMIT_BYTES (MAX_TECHNICIAN_DATA_REQUESTS_LIMIT * sizeof(*(*db_service)->technician_data_list))
#define MAX_CLIENT_DATA_REQUESTS_LIMIT_BYTES (MAX_CLIENT_DATA_REQUESTS_LIMIT * sizeof(*(*db_service)->client_data_list))
#define MAX_REPAIR_REQUESTS_LIMIT_BYTES (MAX_REPAIR_REQUESTS_LIMIT * sizeof(*(*db_service)->repair_list))
#define MAX_SERVICE_REQUESTS_LIMIT_BYTES (MAX_SERVICE_REQUESTS_LIMIT * sizeof(*(*db_service)->service_list))

#define MAX_TECHNICIAN_DATA_REQUESTS_LIMIT_BYTES_A (MAX_TECHNICIAN_DATA_REQUESTS_LIMIT * sizeof(*(db_service->technician_data_list)))
#define MAX_CLIENT_DATA_REQUESTS_LIMIT_BYTES_A (MAX_CLIENT_DATA_REQUESTS_LIMIT * sizeof(*(db_service->client_data_list)))
#define MAX_REPAIR_REQUESTS_LIMIT_BYTES_A (MAX_REPAIR_REQUESTS_LIMIT * sizeof(*(db_service->repair_list)))
#define MAX_SERVICE_REQUESTS_LIMIT_BYTES_A (MAX_SERVICE_REQUESTS_LIMIT * sizeof(*(db_service->service_list)))

static void _db_service_clear(DB_SERVICE *db_service)
{
    DB_DEBUG("_db_service_clear: Clear all db_service query lists ...")
    technician_data_requests_clear(db_service->technician_data_requests);

    DB_DEBUG("_db_service_clear: Cleaning db_service->technician_data_list at %p of size %zu", db_service->technician_data_list, MAX_TECHNICIAN_DATA_REQUESTS_LIMIT_BYTES_A)
    explicit_bzero((void *)db_service->technician_data_list, MAX_TECHNICIAN_DATA_REQUESTS_LIMIT_BYTES_A);
    db_service->technician_data_list_index = -1;

    DB_DEBUG("_db_service_clear: Cleaning db_service->client_data_list at %p of size %zu", db_service->client_data_list, MAX_CLIENT_DATA_REQUESTS_LIMIT_BYTES_A)
    explicit_bzero((void *)db_service->client_data_list, MAX_CLIENT_DATA_REQUESTS_LIMIT_BYTES_A);
    db_service->client_data_list_index = -1;

    DB_DEBUG("_db_service_clear: Cleaning db_service->repair_list at %p of size %zu", db_service->repair_list, MAX_REPAIR_REQUESTS_LIMIT_BYTES_A)
    explicit_bzero((void *)db_service->repair_list, MAX_REPAIR_REQUESTS_LIMIT_BYTES_A);
    db_service->repair_list_index = -1;

    DB_DEBUG("_db_service_clear: Cleaning db_service->service_list at %p of size %zu", db_service->service_list, MAX_SERVICE_REQUESTS_LIMIT_BYTES_A)
    explicit_bzero((void *)db_service->service_list, MAX_SERVICE_REQUESTS_LIMIT_BYTES_A);
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

static int _db_add_client(DB_SERVICE *db_service, CLIENT_DATA *client_data) {
    if (db_service->client_data_list_index > -1) {
        if (MAX_CLIENT_DATA_REQUESTS_LIMIT > db_service->client_data_list_index)
            db_service->client_data_list[(size_t)(db_service->client_data_list_index++)] = client_data;
        else
            return DB_QUERY_ALL_CLIENT_LIST_TECH_OVRFL;
    } else {
        db_service->client_data_list[0] = client_data;
        db_service->client_data_list_index = 1;
    }

    return 0;
}


static int _db_init_query(PGconn *conn)
{
    const Oid query_limit_offset[] = {23, 23};

    DB_BUILD_SQL_BEGIN

        DB_BUILD_SQL(
            QUERY_ALL_TECHNICIANS,
            "select id, name, created_at, email, rules, version, phone_number from technician_data "
            "order by name "
            "limit $1 offset $2",
            query_limit_offset
        )

        DB_BUILD_SQL(
            QUERY_ALL_TECHNICIANS_JSON,
            DB_QUERY_AS_JSON(
                "select id, name, created_at, email, rules, version, phone_number from technician_data "
                "order by name "
                "limit $1 offset $2 ",
            "t"),
            query_limit_offset
        )

        DB_BUILD_SQL(
            QUERY_ALL_CLIENTS,
            "select id, technician_id, created_at, cpf, name, address, district_city, email, phone_number, version from client_data "
            "order by name "
            "limit $1 offset $2 ",
            query_limit_offset
        )

        DB_BUILD_SQL(
            QUERY_ALL_CLIENTS_JSON,
            DB_QUERY_AS_JSON(
                "select id, technician_id, created_at, cpf, name, address, district_city, email, phone_number, version from client_data "
                "order by name "
                "limit $1 offset $2 ",
                "c"
            ),
            query_limit_offset
        )

        DB_BUILD_SQL(
            QUERY_ALL_REPAIR_REQUEST,
            "SELECT id, client_data_id, created_at, status, is_budget, device_problem, "
            "brand_model, serial_number, claimed_defect, observations, monetary_type, "
            "expected_budget_date, expected_delivery_date, labor_bugdet, delivery_date, "
            "warranty, version FROM repair_request "
            "ORDER BY client_data_id "
            "LIMIT $1 OFFSET $2;",
            query_limit_offset
        )

        DB_BUILD_SQL(
            QUERY_ALL_REPAIR_REQUEST_JSON,
            DB_QUERY_AS_JSON(
                "SELECT id, client_data_id, created_at, status, is_budget, device_problem, "
                "brand_model, serial_number, claimed_defect, observations, monetary_type, "
                "expected_budget_date, expected_delivery_date, labor_bugdet, delivery_date, "
                "warranty, version FROM repair_request "
                "ORDER BY client_data_id "
                "LIMIT $1 OFFSET $2", 
            "r"),
            query_limit_offset
        )

        DB_BUILD_SQL(
            QUERY_ALL_SERVICE_REQUEST,
            "select id, repair_request_id, created_at, quantity, unity_price, "
            "description, monetary_type from service_request "
            "order by id "
            "limit $1 offset $2",
            query_limit_offset
        )

        DB_BUILD_SQL(
            QUERY_ALL_SERVICE_REQUEST_JSON,
            DB_QUERY_AS_JSON(
                "select id, repair_request_id, created_at, quantity, unity_price, "
                "description, monetary_type from service_request "
                "order by "
                "id limit $1 offset $2",
                "s"
            ),
            query_limit_offset
        )

    DB_BUILD_SQL_END
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

                if (PQsslInUse((*db_service)->conn)) {
                    const char *protocol = PQsslAttribute((*db_service)->conn, "protocol");
                    const char *cipher = PQsslAttribute((*db_service)->conn, "cipher");
                    DB_INFO("db_service_init: Postgres connection is SECURE: protocol %s and cipher %s\n", protocol?protocol:"Unknown", cipher?cipher:"Unknonw");
                } else
                    DB_WARN("db_service_init: Postgres connection is INSECURE\n");

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

static time_t _get_pg_time(PGresult *res, int row, int col)
{
    int64_t ret;
    if (!PQgetisnull(res, row, col)) {
        if ((PQftype(res, col) == TIMESTAMPTZOID) && (PQgetlength(res, row, col) == (int)sizeof(ret))) {
            memcpy(&ret, PQgetvalue(res, row, col), sizeof(ret));
            ret = be64toh(ret); // BE to LE
            ret /= 1000000LL; // Convert microsseconds to seconds ...
            ret += 946684800LL; // Add 30 Years in seconds (postgres initial timestamp 2000)
            if (ret > 0)
                return ret;
            
            DB_WARN("_get_pg_time: Negative value of timestamp tz")
        } else {
            DB_WARN("_get_pg_time: Invalid timestamp tz or invalid type size")
        }

        return 0;
    }

    DB_WARN("_get_pg_time: Invalid NULL date time")
    return 0;
}

DB_STRING *_get_pg_string(DB_STRING *db_string_ptr, PGresult *res, int row, int col, Oid string_type)
{
    if (!PQgetisnull(res, row, col)) {
        int len = PQgetlength(res, row, col);
        if (len >= 0) {
            Oid type = PQftype(res, col);
            if ((type == string_type)) {
                db_string_ptr->str_len = (size_t)len;
                db_string_ptr->str = PQgetvalue(res, row, col);

                return db_string_ptr;
            }
            DB_WARN("_get_pg_string: Invalid string type %d. Was expected %d", type, string_type)
        }
    }

    return NULL;
}

static int32_t _get_pg_i32(PGresult *res, int row, int col)
{
    int32_t ret;
    if (!PQgetisnull(res, row, col)) {
        if ((PQftype(res, col) == INT4OID) && (PQgetlength(res, row, col) == (int)sizeof(ret))) {
            memcpy(&ret, PQgetvalue(res, row, col), sizeof(ret));
            return (int32_t)ntohl(ret);
        }
        DB_WARN("_get_pg_i32: INVALID TYPE OF INT32. Returning -2 ...")
        return -2;
    }

    DB_WARN("_get_pg_i32: NULL INT32. Returning -1 ...")
    return -1;
}

int db_service_load_technicians_json(char **json_result, size_t *json_result_len, DB_SERVICE *db_service, uint32_t limit, uint32_t offset)
{
    if(db_service) {
        if ((json_result != NULL) && (*json_result == NULL)) {
            if (json_result_len)
                *json_result_len = 0;

            uint32_t limit_be  = htonl((uint32_t)limit);
            uint32_t offset_be = htonl((uint32_t)offset);

            const char *param_values[] = { (const char *)&limit_be, (const char *)&offset_be };
            int param_lengths[]        = { (int)sizeof(limit_be), (int)sizeof(offset_be) };
            int param_formats[]        = { 1, 1 };

            DB_DEBUG("db_service_load_technicians_json: Check PostgreSQL connection ...")
            DB_SERVICE_CHECK_CONN(
                DB_SERVICE_CLOSED_OR_OFFLINE_OR_NOT_AVAILABLE,
                "Load technicians query as JSON failed"
            )

            DB_DEBUG("db_service_load_technicians_json: Execute query ...")
            PGresult *res = PQexecPrepared(
                db_service->conn,
                QUERY_ALL_TECHNICIANS_JSON, 
                2,
                param_values,
                param_lengths,
                param_formats,
                0 // 0 = string value as result | 1 = for binary
            );
            DB_DEBUG("db_service_load_technicians_json: Check results ...")

            if (res) {
                if (PQresultStatus(res) == PGRES_TUPLES_OK) {
                    const char *json_string = DB_EMPTY_JSON_ARRAY;
                    int json_len_tmp = (int)DB_EMPTY_JSON_ARRAY_LEN;

                    if (!PQgetisnull(res, 0, 0)) {
                        // SUCCESS
                        DB_DEBUG("db_service_load_technicians_json: Cleaning all last requests ...")

                        _db_service_clear(db_service);

                        json_string = PQgetvalue(res, 0, 0);
                        json_len_tmp = PQgetlength(res, 0, 0);
                        DB_DEBUG("db_service_load_technicians_json: Begin JSON string parsing at pointer %p of size %d\n", json_string, json_len_tmp)

                        if (json_len_tmp <= 0) {
                            json_string = DB_EMPTY_JSON_ARRAY;
                            json_len_tmp = DB_EMPTY_JSON_ARRAY_LEN;
                        }
db_service_load_technicians_json_cpy:
                        DB_DEBUG("db_service_load_technicians_json: Begin allocation of new %d bytes to copy Postgres result", json_len_tmp + 1)
                        int err = _db_alloc_align((void **)json_result, (size_t)(json_len_tmp + 1));

                        if (err == 0) {
                            memcpy((void *)*json_result, (void *)json_string, (size_t)json_len_tmp);
                            (*json_result)[(size_t)json_len_tmp] = 0;

                            if (json_len_tmp)
                                *json_result_len = (size_t)json_len_tmp;

                            DB_DEBUG("db_service_load_technicians_json: Postgres copy %d bytes of JSON success from %p to %p", json_len_tmp, json_string, *json_result)
                        } else {
                            DB_ERROR("db_service_load_technicians_json: Unexpected _db_alloc_align error %d", err)
                            set_db_service_error(
                                db_service,
                                DB_QUERY_ALL_TECHNICIANS_JSON_ALLOCATION_COPY_BLOCK,
                                "JSON Postgres copy allocation error %d", err
                            );        
                        }
                    } else {
                        DB_DEBUG("db_service_load_technicians_json: Postgres return empty query ...")
                        goto db_service_load_technicians_json_cpy;
                    }
                } else {
                    const char *sqlstate = PQresultErrorField(res, PG_DIAG_SQLSTATE);
                    set_db_service_error(
                        db_service,
                        DB_QUERY_ALL_TECHNICIANS_EXECUTION_JSON_QUERY_ERROR,
                        "Execute load all technician query as JSON failed. SQL state: %s. Postgres message %s",
                        ((sqlstate)?sqlstate:"None"),
                        PQresultErrorMessage(res)
                    );
                }

                PQclear(res);
            } else
                set_db_service_error(
                    db_service,
                    DB_QUERY_ALL_TECHNICIANS_JSON_EXECUTION_OUT_OF_MEMORY,
                    "Error. Execute load all technician query as JSON failed. Out of memory"
                );
        } else {
            set_db_service_error(
                db_service,
                DB_QUERY_ALL_TECHNICIANS_JSON_INVALID_JSON_POINTER,
                "Invalid JSON pointer"
            );
        }

        DB_DEBUG("db_service_load_technicians_json: Query result status: %d", db_service->err)
        DB_SERVICE_RETURN
    }

    DB_ERROR("db_service_load_technicians_json: Null pointer")
    DB_DEBUG("db_service_load_technicians_json: Invalid pointer")
    DB_SERVICE_RETURN
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
                        //const char *name = PQgetvalue(res, i, 1);
                        DB_STRING name;
                        time_t created_at = _get_pg_time(res, i, 2);
                        //const char *email = PQgetvalue(res, i, 3);
                        DB_STRING email;
                        int32_t rules = _get_pg_i32(res, i, 4);
                        int32_t version = _get_pg_i32(res, i, 5);
                        //const char *phone_number = PQgetvalue(res, i, 6);
                        DB_STRING phone_number;
#ifdef SOLDA_DEBUG
                        char buffer[64];
                        DB_DEBUG("Created at: %s", db_time(&buffer[0], sizeof(buffer), &created_at))
#endif
                        /*
                        DB_DEBUG("Technician ID: %u", id)
                        DB_DEBUG("Technician name: %s", name)
                        DB_DEBUG("Technician email: %s", email)
                        DB_DEBUG("Technician rules: %d", rules)
                        DB_DEBUG("Technician version: %d", version)
                        */
                        if ((err = _db_add_technician(db_service, out)) == 0) {
                            err = TECHNICIAN_EXECUTE_READ(
                                out, 
                                TECHNICIAN_READ_ID(id),
                                TECHNICIAN_READ_NAME(_get_pg_string(&name, res, i, 1, VARCHAROID)),
                                TECHNICIAN_READ_CREATED_AT(created_at),
                                TECHNICIAN_READ_EMAIL(_get_pg_string(&email, res, i, 3, VARCHAROID)),
                                TECHNICIAN_READ_RULES(rules),
                                TECHNICIAN_READ_VERSION(version),
                                TECHNICIAN_READ_PHONE_NUMBER(_get_pg_string(&phone_number, res, i, 6, VARCHAROID))
                            )
                            //out->flag = TECHNICIAN_READ_FROM_DATABASE;
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

    DB_ERROR("db_service_load_technicians: Null pointer")
    DB_DEBUG("db_service_load_technicians: Invalid pointer")
    DB_SERVICE_RETURN
}

int db_service_load_clients(DB_SERVICE *db_service, uint32_t limit, uint32_t offset)
{
    DB_DEBUG("Entering db_service_load_clients ...")
    if (db_service) {

        uint32_t limit_be  = htonl((uint32_t)limit);
        uint32_t offset_be = htonl((uint32_t)offset);

        const char *param_values[] = { (const char *)&limit_be, (const char *)&offset_be };
        int param_lengths[]        = { (int)sizeof(limit_be), (int)sizeof(offset_be) };
        int param_formats[]        = { 1, 1 };

        DB_DEBUG("db_service_load_clients: Check PostgreSQL connection ...")
        DB_SERVICE_CHECK_CONN(
            DB_SERVICE_CLOSED_OR_OFFLINE_OR_NOT_AVAILABLE,
            "Load clients query failed"
        )

        DB_DEBUG("db_service_load_clients: Execute query ...")
        PGresult *res = PQexecPrepared(
            db_service->conn,
            QUERY_ALL_CLIENTS, 
            2,
            param_values,
            param_lengths,
            param_formats,
            1 // 0 = string value as result | 1 = for binary
        );
        DB_DEBUG("db_service_load_clients: Check results ...")
        if (res) {
            if (PQresultStatus(res) == PGRES_TUPLES_OK) {
                // SUCCESS
                DB_DEBUG("Cleaning all last requests for clients...")
                _db_service_clear(db_service);

                DB_DEBUG("Begin copy to allocated registries for clients query")

                int err;
                size_t index_technician;
                size_t index_client;
                TECHNICIAN_DATA *out_technician = NULL;
                CLIENT_DATA *out_client;
                int rows = PQntuples(res);

                // We must only acquire one technician data to store array of client (CLIENT QUERY ONLY)
                if ((err = technician_acquire_technician_data_from_array(&index_technician, &out_technician, db_service->technician_data_requests)) == 0) {
                    for (int i = 0; i < rows; i++) {
                        out_client = NULL;
                        if ((err = technician_acquire_client_data_from_array(&index_client, &out_client, index_technician, db_service->technician_data_requests)) == 0) {
//id, technician_id, created_at, cpf,
//name, address, district_city, email, 
//phone_number, version
                            int32_t id = _get_pg_i32(res, i, 0);
                            int32_t technician_id = _get_pg_i32(res, i, 1);
                            time_t created_at = _get_pg_time(res, i, 2);
                            //const char *cpf = PQgetvalue(res, i, 3);
                            DB_STRING cpf;
                            //const char *name = PQgetvalue(res, i, 4);
                            DB_STRING name;
                            //const char *address = PQgetvalue(res, i, 5);
                            DB_STRING address;
                            //const char *district_city = PQgetvalue(res, i, 6);
                            DB_STRING district_city;
                            //const char *email = PQgetvalue(res, i, 7);
                            DB_STRING email;
                            //const char *phone_number = PQgetvalue(res, i, 8);
                            DB_STRING phone_number;
                            int32_t version = _get_pg_i32(res, i, 9);

    #ifdef SOLDA_DEBUG
                            char buffer[64];
                            DB_DEBUG("Created at: %s", db_time(&buffer[0], sizeof(buffer), &created_at))
    #endif
                            /*
                            DB_DEBUG("Client ID: %u", id)
                            DB_DEBUG("Client Technician ID: %u", technician_id)
                            DB_DEBUG("Client cpf: %s", cpf)
                            DB_DEBUG("Client name: %s", name)
                            DB_DEBUG("Client address: %s", address)
                            DB_DEBUG("Client district_city: %s", district_city)
                            DB_DEBUG("Client email: %s", email)
                            DB_DEBUG("Client phone number: %s", phone_number)
                            DB_DEBUG("Client version: %u", version)
                            */
                            if ((err = _db_add_client(db_service, out_client)) == 0) {
                                err = CLIENT_EXECUTE_READ(
                                    out_client, 
                                    CLIENT_READ_ID(id),
                                    CLIENT_READ_TECHNICIAN_ID(technician_id),
                                    CLIENT_READ_CREATED_AT(created_at),
                                    CLIENT_READ_CPF(_get_pg_string(&cpf, res, i, 3, VARCHAROID)),
                                    CLIENT_READ_NAME(_get_pg_string(&name, res, i, 4, VARCHAROID)),
                                    CLIENT_READ_ADDRESS(_get_pg_string(&address, res, i, 5, VARCHAROID)),
                                    CLIENT_READ_DISTRICT_CITY(_get_pg_string(&district_city, res, i, 6, VARCHAROID)),
                                    CLIENT_READ_EMAIL(_get_pg_string(&email, res, i, 7, VARCHAROID)),
                                    CLIENT_READ_PHONE_NUMBER(_get_pg_string(&phone_number, res, i, 8, VARCHAROID)),
                                    CLIENT_READ_VERSION(version)
                                )
                                //out_client->flag = CLIENT_DATA_READ_FROM_DATABASE;
                                if (err) {
                                    set_db_service_error(
                                        db_service,
                                        err,
                                        "Fail CLIENT_EXECUTE_READ @ db_service_load_clients: at index %d. Unable to add to temporary list",
                                        i
                                    );
                                    _db_service_clear(db_service);
                                    break;
                                }
                            } else {
                                set_db_service_error(
                                    db_service,
                                    err,
                                    "Fail _db_add_client @ db_service_load_clients: at index %d. Unable to add to temporary list",
                                    i
                                );
                                _db_service_clear(db_service);
                                break;
                            }
                        } else {
                            set_db_service_error(
                                db_service,
                                err,
                                "Fail technician_acquire_client_data_from_array @ db_service_load_clients: at index %d",
                                i
                            );
                            _db_service_clear(db_service);
                            break;
                        }
                    }
                    DB_DEBUG("Ending copying to allocated registries")
                } else {
                    set_db_service_error(
                        db_service,
                        err,
                        "db_service_load_clients: Fail technician_acquire_technician_data_from_array @ db_service_load_clients"
                    );
                    _db_service_clear(db_service);                        
                }
            } else {
                const char *sqlstate = PQresultErrorField(res, PG_DIAG_SQLSTATE);
                set_db_service_error(
                    db_service,
                    DB_QUERY_ALL_TECHNICIANS_EXECUTION_QUERY_ERROR,
                    "Execute load all clients query failed. SQL state: %s. Postgres message %s",
                    ((sqlstate)?sqlstate:"None"),
                    PQresultErrorMessage(res)
                );
            }

            PQclear(res);
        } else
            set_db_service_error(
                db_service,
                DB_QUERY_ALL_CLIENTS_EXECUTION_OUT_OF_MEMORY,
                "Error. Execute load all clients query failed. Out of memory"
            );

        DB_DEBUG("db_service_load_clients: Query result status: %d", db_service->err)
        DB_SERVICE_RETURN
    }

    DB_ERROR("db_service_load_clients: Null pointer")
    DB_DEBUG("db_service_load_clients: Invalid pointer")
    DB_SERVICE_RETURN
}

int db_service_load_clients_json(char **json_result, size_t *json_result_len, DB_SERVICE *db_service, uint32_t limit, uint32_t offset)
{
    if(db_service) {
        if ((json_result != NULL) && (*json_result == NULL)) {
            if (json_result_len)
                *json_result_len = 0;

            uint32_t limit_be  = htonl((uint32_t)limit);
            uint32_t offset_be = htonl((uint32_t)offset);

            const char *param_values[] = { (const char *)&limit_be, (const char *)&offset_be };
            int param_lengths[]        = { (int)sizeof(limit_be), (int)sizeof(offset_be) };
            int param_formats[]        = { 1, 1 };

            DB_DEBUG("db_service_load_clients_json: Check PostgreSQL connection ...")
            DB_SERVICE_CHECK_CONN(
                DB_SERVICE_CLOSED_OR_OFFLINE_OR_NOT_AVAILABLE,
                "Load clients query as JSON failed"
            )

            DB_DEBUG("db_service_load_clients_json: Execute query ...")
            PGresult *res = PQexecPrepared(
                db_service->conn,
                QUERY_ALL_CLIENTS_JSON, 
                2,
                param_values,
                param_lengths,
                param_formats,
                0 // 0 = string value as result | 1 = for binary
            );
            DB_DEBUG("db_service_load_clients_json: Check results ...")

            if (res) {
                if (PQresultStatus(res) == PGRES_TUPLES_OK) {
                    const char *json_string = DB_EMPTY_JSON_ARRAY;
                    int json_len_tmp = (int)DB_EMPTY_JSON_ARRAY_LEN;

                    if (!PQgetisnull(res, 0, 0)) {
                        // SUCCESS
                        DB_DEBUG("db_service_load_clients_json: Cleaning all last requests ...")

                        _db_service_clear(db_service);

                        json_string = PQgetvalue(res, 0, 0);
                        json_len_tmp = PQgetlength(res, 0, 0);
                        DB_DEBUG("db_service_load_clients_json: Begin JSON string parsing at pointer %p of size %d\n", json_string, json_len_tmp)

                        if (json_len_tmp <= 0) {
                            json_string = DB_EMPTY_JSON_ARRAY;
                            json_len_tmp = DB_EMPTY_JSON_ARRAY_LEN;
                        }
db_service_load_clients_json_cpy:
                        DB_DEBUG("db_service_load_clients_json: Begin allocation of new %d bytes to copy Postgres result", json_len_tmp + 1)
                        int err = _db_alloc_align((void **)json_result, (size_t)(json_len_tmp + 1));

                        if (err == 0) {
                            memcpy((void *)*json_result, (void *)json_string, (size_t)json_len_tmp);
                            (*json_result)[(size_t)json_len_tmp] = 0;

                            if (json_len_tmp)
                                *json_result_len = (size_t)json_len_tmp;

                            DB_DEBUG("db_service_load_clients_json: Postgres copy %d bytes of JSON success from %p to %p", json_len_tmp, json_string, *json_result)
                        } else {
                            DB_ERROR("db_service_load_clients_json: Unexpected _db_alloc_align error %d", err)
                            set_db_service_error(
                                db_service,
                                DB_QUERY_ALL_CLIENTS_JSON_ALLOCATION_COPY_BLOCK,
                                "JSON Postgres copy allocation error %d", err
                            );        
                        }
                    } else {
                        DB_DEBUG("db_service_load_clients_json: Postgres return empty query ...")
                        goto db_service_load_clients_json_cpy;
                    }
                } else {
                    const char *sqlstate = PQresultErrorField(res, PG_DIAG_SQLSTATE);
                    set_db_service_error(
                        db_service,
                        DB_QUERY_ALL_CLIENTS_EXECUTION_JSON_QUERY_ERROR,
                        "Execute load all clients query as JSON failed. SQL state: %s. Postgres message %s",
                        ((sqlstate)?sqlstate:"None"),
                        PQresultErrorMessage(res)
                    );
                }

                PQclear(res);
            } else
                set_db_service_error(
                    db_service,
                    DB_QUERY_ALL_CLIENTS_JSON_EXECUTION_OUT_OF_MEMORY,
                    "Error. Execute load all clients query as JSON failed. Out of memory"
                );
        } else {
            set_db_service_error(
                db_service,
                DB_QUERY_ALL_CLIENTS_JSON_INVALID_JSON_POINTER,
                "db_service_load_clients_json: Invalid JSON pointer"
            );
        }

        DB_DEBUG("db_service_load_clients_json: Query result status: %d", db_service->err)
        DB_SERVICE_RETURN
    }

    DB_ERROR("db_service_load_clients_json: Null pointer")
    DB_DEBUG("db_service_load_clients_json: Invalid pointer")
    DB_SERVICE_RETURN
}

/*
int db_service_load_repair_requests(DB_SERVICE *db_service, uint32_t limit, uint32_t offset)
{
    TODO IMPLEMENT IT
}
*/

int db_service_load_repair_requests_json(char **json_result, size_t *json_result_len, DB_SERVICE *db_service, uint32_t limit, uint32_t offset)
{
    if(db_service) {
        if ((json_result != NULL) && (*json_result == NULL)) {
            if (json_result_len)
                *json_result_len = 0;

            uint32_t limit_be  = htonl((uint32_t)limit);
            uint32_t offset_be = htonl((uint32_t)offset);

            const char *param_values[] = { (const char *)&limit_be, (const char *)&offset_be };
            int param_lengths[]        = { (int)sizeof(limit_be), (int)sizeof(offset_be) };
            int param_formats[]        = { 1, 1 };

            DB_DEBUG("db_service_load_repair_requests_json: Check PostgreSQL connection ...")
            DB_SERVICE_CHECK_CONN(
                DB_SERVICE_CLOSED_OR_OFFLINE_OR_NOT_AVAILABLE,
                "Load repair requests query as JSON failed"
            )

            DB_DEBUG("db_service_load_repair_requests_json: Execute query ...")
            PGresult *res = PQexecPrepared(
                db_service->conn,
                QUERY_ALL_REPAIR_REQUEST_JSON, 
                2,
                param_values,
                param_lengths,
                param_formats,
                0 // 0 = string value as result | 1 = for binary
            );
            DB_DEBUG("db_service_load_repair_requests_json: Check results ...")

            if (res) {
                if (PQresultStatus(res) == PGRES_TUPLES_OK) {
                    const char *json_string = DB_EMPTY_JSON_ARRAY;
                    int json_len_tmp = (int)DB_EMPTY_JSON_ARRAY_LEN;

                    if (!PQgetisnull(res, 0, 0)) {
                        // SUCCESS
                        DB_DEBUG("db_service_load_repair_requests_json: Cleaning all last requests ...")

                        _db_service_clear(db_service);

                        json_string = PQgetvalue(res, 0, 0);
                        json_len_tmp = PQgetlength(res, 0, 0);
                        DB_DEBUG("db_service_load_repair_requests_json: Begin JSON string parsing at pointer %p of size %d\n", json_string, json_len_tmp)

                        if (json_len_tmp <= 0) {
                            json_string = DB_EMPTY_JSON_ARRAY;
                            json_len_tmp = DB_EMPTY_JSON_ARRAY_LEN;
                        }

db_service_load_repair_requests_json_cpy:
                        DB_DEBUG("db_service_load_repair_requests_json: Begin allocation of new %d bytes to copy Postgres result", json_len_tmp + 1)
                        int err = _db_alloc_align((void **)json_result, (size_t)(json_len_tmp + 1));

                        if (err == 0) {
                            memcpy((void *)*json_result, (void *)json_string, (size_t)json_len_tmp);
                            (*json_result)[(size_t)json_len_tmp] = 0;

                            if (json_len_tmp)
                                *json_result_len = (size_t)json_len_tmp;

                            DB_DEBUG("db_service_load_repair_requests_json: Postgres copy %d bytes of JSON success from %p to %p", json_len_tmp, json_string, *json_result)
                        } else {
                            DB_ERROR("db_service_load_repair_requests_json: Unexpected _db_alloc_align error %d", err)
                            set_db_service_error(
                                db_service,
                                DB_QUERY_ALL_REPAIR_REQUEST_JSON_ALLOCATION_COPY_BLOCK,
                                "JSON Postgres copy allocation error %d", err
                            );        
                        }
                    } else {
                        DB_DEBUG("db_service_load_repair_requests_json: Postgres returns emtpy query ...")
                        goto db_service_load_repair_requests_json_cpy;
                    }
                } else {
                    const char *sqlstate = PQresultErrorField(res, PG_DIAG_SQLSTATE);
                    set_db_service_error(
                        db_service,
                        DB_QUERY_ALL_REPAIR_REQUESTS_EXECUTION_JSON_QUERY_ERROR,
                        "Execute load all repair requests query as JSON failed. SQL state: %s. Postgres message %s",
                        ((sqlstate)?sqlstate:"None"),
                        PQresultErrorMessage(res)
                    );
                }

                PQclear(res);
            } else
                set_db_service_error(
                    db_service,
                    DB_QUERY_ALL_REPAIR_REQUESTS_JSON_EXECUTION_OUT_OF_MEMORY,
                    "Error. Execute load all repair requests query as JSON failed. Out of memory"
                );
        } else {
            set_db_service_error(
                db_service,
                DB_QUERY_ALL_REPAIR_REQUESTS_JSON_INVALID_JSON_POINTER,
                "db_service_load_repair_requests_json: Invalid JSON pointer"
            );
        }

        DB_DEBUG("db_service_load_repair_requests_json: Query result status: %d", db_service->err)
        DB_SERVICE_RETURN
    }

    DB_ERROR("db_service_load_repair_requests_json: Null pointer")
    DB_DEBUG("db_service_load_repair_requests_json: Invalid pointer")
    DB_SERVICE_RETURN
}

/*
int db_service_load_service_requests(DB_SERVICE *db_service, uint32_t limit, uint32_t offset)
{
    TODO IMPLEMENT IT
}
*/

int db_service_load_service_requests_json(char **json_result, size_t *json_result_len, DB_SERVICE *db_service, uint32_t limit, uint32_t offset)
{
    if(db_service) {
        if ((json_result != NULL) && (*json_result == NULL)) {
            if (json_result_len)
                *json_result_len = 0;

            uint32_t limit_be  = htonl((uint32_t)limit);
            uint32_t offset_be = htonl((uint32_t)offset);

            const char *param_values[] = { (const char *)&limit_be, (const char *)&offset_be };
            int param_lengths[]        = { (int)sizeof(limit_be), (int)sizeof(offset_be) };
            int param_formats[]        = { 1, 1 };

            DB_DEBUG("db_service_load_service_requests_json: Check PostgreSQL connection ...")
            DB_SERVICE_CHECK_CONN(
                DB_SERVICE_CLOSED_OR_OFFLINE_OR_NOT_AVAILABLE,
                "Load service requests query as JSON failed"
            )

            DB_DEBUG("db_service_load_service_requests_json: Execute query ...")
            PGresult *res = PQexecPrepared(
                db_service->conn,
                QUERY_ALL_SERVICE_REQUEST_JSON, 
                2,
                param_values,
                param_lengths,
                param_formats,
                0 // 0 = string value as result | 1 = for binary
            );
            DB_DEBUG("db_service_load_service_requests_json: Check results ...")

            if (res) {
                if (PQresultStatus(res) == PGRES_TUPLES_OK) {
                    const char *json_string = DB_EMPTY_JSON_ARRAY;
                    int json_len_tmp = (int)DB_EMPTY_JSON_ARRAY_LEN;

                    if (!PQgetisnull(res, 0, 0)) {
                        // SUCCESS
                        DB_DEBUG("db_service_load_service_requests_json: Cleaning all last requests ...")

                        _db_service_clear(db_service);

                        json_string = PQgetvalue(res, 0, 0);
                        json_len_tmp = PQgetlength(res, 0, 0);
                        DB_DEBUG("db_service_load_service_requests_json: Begin JSON string parsing at pointer %p of size %d\n", json_string, json_len_tmp)

                        if (json_len_tmp <= 0) {
                            json_string = DB_EMPTY_JSON_ARRAY;
                            json_len_tmp = DB_EMPTY_JSON_ARRAY_LEN;
                        }

db_service_load_repair_requests_json_cpy:
                        DB_DEBUG("db_service_load_service_requests_json: Begin allocation of new %d bytes to copy Postgres result", json_len_tmp + 1)
                        int err = _db_alloc_align((void **)json_result, (size_t)(json_len_tmp + 1));

                        if (err == 0) {
                            memcpy((void *)*json_result, (void *)json_string, (size_t)json_len_tmp);
                            (*json_result)[(size_t)json_len_tmp] = 0;

                            if (json_len_tmp)
                                *json_result_len = (size_t)json_len_tmp;

                            DB_DEBUG("db_service_load_service_requests_json: Postgres copy %d bytes of JSON success from %p to %p", json_len_tmp, json_string, *json_result)
                        } else {
                            DB_ERROR("db_service_load_service_requests_json: Unexpected _db_alloc_align error %d", err)
                            set_db_service_error(
                                db_service,
                                DB_QUERY_ALL_SERVICE_REQUEST_JSON_ALLOCATION_COPY_BLOCK,
                                "JSON Postgres copy allocation error %d", err
                            );        
                        }
                    } else {
                        DB_DEBUG("db_service_load_service_requests_json: Postgres returns emtpy query ...")
                        goto db_service_load_repair_requests_json_cpy;
                    }
                } else {
                    const char *sqlstate = PQresultErrorField(res, PG_DIAG_SQLSTATE);
                    set_db_service_error(
                        db_service,
                        DB_QUERY_ALL_SERVICE_REQUESTS_EXECUTION_JSON_QUERY_ERROR,
                        "Execute load all service requests query as JSON failed. SQL state: %s. Postgres message %s",
                        ((sqlstate)?sqlstate:"None"),
                        PQresultErrorMessage(res)
                    );
                }

                PQclear(res);
            } else
                set_db_service_error(
                    db_service,
                    DB_QUERY_ALL_SERVICE_REQUESTS_JSON_EXECUTION_OUT_OF_MEMORY,
                    "Error. Execute load all service requests query as JSON failed. Out of memory"
                );
        } else {
            set_db_service_error(
                db_service,
                DB_QUERY_ALL_SERVICE_REQUESTS_JSON_INVALID_JSON_POINTER,
                "db_service_load_service_requests_json: Invalid JSON pointer"
            );
        }

        DB_DEBUG("db_service_load_service_requests_json: Query result status: %d", db_service->err)
        DB_SERVICE_RETURN
    }

    DB_ERROR("db_service_load_service_requests_json: Null pointer")
    DB_DEBUG("db_service_load_service_requests_json: Invalid pointer")
    DB_SERVICE_RETURN
}
