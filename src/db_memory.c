#include <stdlib.h>
#include <stdbool.h>
#include <db_errors.h>

inline int db_alloc(void **mem, size_t mem_size)
{
    if ((mem != NULL) && (*mem == NULL) && (mem_size > 0)) {
        int err = posix_memalign(mem, 64, mem_size);

        if (err = 0)
            memset(*mem, 0, mem_size);

        return err;
    }

    return DB_UNABLE_TO_ALLOCATE_USER_MEMORY;
}

inline void db_free(void **mem)
{
    if ((mem != NULL) && (*mem != NULL)) {
        free(*mem);
        *mem = NULL;
    }
}

inline void db_clear_and_free(void **mem, size_t mem_size)
{
    if ((mem != NULL) && (*mem != NULL)) {
        if (mem_size)
            memset(*mem, 0, mem_size);

        free(*mem);
        *mem = NULL;
    }
}
