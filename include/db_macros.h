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

#define ACQUIRE_FUNC(func, arra_grow_func, parent_type, child_type, field) \
int func(size_t *index, child_type **out, parent_type *in) \
{ \
    *out = NULL; \
    if (in != NULL) { \
\
        if (in->field.n >= in->field.array_max_len) { \
            int err = arra_grow_func##_grow(&in->field, 1); \
            if (err) \
                return err; \
        } \
\
        if (index) \
            *index = in->field.n; \
\
        *out = &(in->field.array[in->field.n++]); \
\
        return 0; \
    } \
\
    return DB_UNABLE_TO_ADD_##child_type; \
}

#endif