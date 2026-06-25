#ifndef DB_MACROS_H
 #define DB_MACROS_H

#define DB_ARRAY_FREE(arr) \
    if (arr->array) { \
        explicit_bzero((void *)arr->array, arr->array_max_len * sizeof(*(arr->array))); \
        free((void *)arr->array); \
        arr->array = NULL; \
    }

#define DB_ARRAY_CLEAR(arr) \
    if (arr->array) { \
        explicit_bzero((void *)arr->array, arr->array_max_len * sizeof(*(arr->array))); \
        arr->n = 0; \
    }

#define DB_NULLABLE_ARRAY_CLEAR(arr) \
    if ((arr != NULL) && (arr->array != NULL)) { \
        explicit_bzero((void *)arr->array, arr->array_max_len * sizeof(*(arr->array))); \
        arr->n = 0; \
    }

#define DB_CLEAR_ELEMENT(el) \
    if (el) {\
        explicit_bzero((void *)el, sizeof(*(el))); \
    }

#define DB_CLEAR_NON_NULL_ELEMENT(el) explicit_bzero((void *)el, sizeof(*(el)));

#define GROW_ARRAY_FUNC(func_name, type) \
static int func_name##_grow(type **requests, size_t plus_n) \
{ \
    if (plus_n > 0) { \
        plus_n += (*requests)->array_max_len; \
\
        DB_ALIGN_VEC_LENGTH(plus_n, MIN_##type##_INITIAL) \
\
        if (MAX_##type##_LIMIT >= plus_n) { \
            type *new = NULL, *current; \
\
            if (db_alloc((void **)&new, plus_n * sizeof(type))) \
                return DB_UNABLE_TO_GROW_AND_MOVE_##type; \
\
            current = (*requests)->array; \
\
            memcpy((void *)new, (void *)current, (*requests)->n * sizeof(*new)); \
\
            explicit_bzero((void *)current, (*requests)->array_max_len * sizeof(*current)); \
            free((void *)current); \
\
            (*requests)->array = new; \
            (*requests)->array_max_len = plus_n; \
\
            return 0; \
        } \
    } \
\
    return DB_UNABLE_TO_GROW_##type; \
}

/*static int repair_request_grow(REPAIR_REQUESTS **requests, size_t plus_n)
{
    if (plus_n > 0) {
        plus_n += (*requests)->array_max_len;

        DB_ALIGN_VEC_LENGTH(plus_n, MIN_REPAIR_REQUEST_INITIAL)

        if (MAX_REPAIR_REQUESTS_LIMIT >= plus_n) {
            REPAIR_REQUEST *new = NULL, *current;

            if (db_alloc((void **)&new, plus_n * sizeof(REPAIR_REQUEST)))
                return DB_UNABLE_TO_GROW_AND_MOVE_REPAIR_REQUESTS;

            current = (*requests)->array;

            memcpy((void *)new, (void *)current, (*requests)->n * sizeof(*new));

            db_clear_and_free((void **)&current, (*requests)->array_max_len * sizeof(*current));

            (*requests)->array = new;
            (*requests)->array_max_len = plus_n;

            new = NULL;

            return 0;
        }
    }

    return DB_UNABLE_TO_GROW_REPAIR_REQUESTS;
}*/

#endif