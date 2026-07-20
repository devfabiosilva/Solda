#ifndef DB_SERVICE_MACROS_H
 #define DB_SERVICE_MACROS_H

#define DB_SERVICE_CHECK_CONN(error, message_prefix) \
if (PQstatus(db_service->conn) != CONNECTION_OK) { \
    set_db_service_error( \
        db_service, \
        error, \
        message_prefix ". PostgreSQL connection is not active or is closed." \
    ); \
    DB_SERVICE_RETURN \
}

#define DB_BUILD_SQL_BEGIN \
    int err = 0; \
    PGresult *res; \
    DB_DEBUG("Entering _db_init_query ...")

#define DB_BUILD_SQL(query_name, text, param_types_oid) \
    DB_DEBUG("Preparing query \"" #query_name "\" ...") \
    res = PQprepare( \
        conn, query_name, \
        text, \
        (int)(sizeof(param_types_oid)/sizeof(const Oid)), param_types_oid \
    ); \
    DB_DEBUG("Check query \"" #query_name  "\" ...") \
\
    if (res) { \
        if (PQresultStatus(res) != PGRES_COMMAND_OK) { \
            DB_ERROR("_db_init_query: PQresultStatus returned error for query " #query_name " with message: %s", PQresultErrorMessage(res)) \
            err = DB_##query_name##_FAILED; \
        } \
\
        PQclear(res); \
\
        if (err) \
            return err; \
    } else \
        return DB_SERVICE_PREPARE_##query_name##_DATA_OUT_OF_MEMORY;

#define DB_BUILD_SQL_END \
    DB_DEBUG("_db_init_query status: %d ...", err) \
    return err;

#endif