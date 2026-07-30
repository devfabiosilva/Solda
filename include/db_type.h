#ifndef DB_TYPE_H
 #define DB_TYPE_H

#include <stddef.h>

typedef struct db_type_string_t {
    const char *str;
    size_t str_len;
} DB_STRING;

#endif