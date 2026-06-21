#ifndef DB_MEMORY_H
 #define DB_MEMORY_H

#include <stdlib.h>

int db_alloc(void **, size_t);
void db_free(void **);
void db_clear_and_free(void **, size_t);

#define DB_ALIGN_VEC_LENGTH(vec_len, blk_sz) \
    if (vec_len & (blk_sz - 1)) { \
        vec_len += blk_sz; \
        vec_len &= ~(blk_sz - 1); \
    }

#endif
