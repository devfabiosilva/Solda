#ifndef DB_LOG_H
#define DB_LOG_H

#include <stddef.h>

typedef enum db_log_type_e { LOG_INFO, LOG_WARN, LOG_ERROR, LOG_DEBUG } DB_LOG;

void _db_log(DB_LOG, const char *, ...);

#define DB_INFO(...) _db_log(LOG_INFO, __VA_ARGS__);

#define DB_WARN(...) _db_log(LOG_WARN, __VA_ARGS__);

#ifdef SOLDA_DEBUG
void _db_log_error(const char *, const char *, int, const char *fmt, ...);

#define DB_ERROR(...) _db_log_error(__func__, __FILE__, __LINE__, __VA_ARGS__);

#define DB_DEBUG(...) _db_log(LOG_DEBUG, __VA_ARGS__);

#else
#define DB_ERROR(...) _db_log(LOG_ERROR, __VA_ARGS__);

#define DB_DEBUG(...)
#endif

#endif
