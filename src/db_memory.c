#include <db_errors.h>
#include <db_log.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

int _db_alloc_align(void **mem, size_t mem_size) {
  return posix_memalign(mem, 64, mem_size);
}

int db_alloc(void **mem, size_t mem_size) {
  DB_DEBUG("db_alloc: Init alloc block of size %zu bytes", mem_size)
  if ((mem != NULL) && (*mem == NULL) && (mem_size > 0)) {
    int err = posix_memalign(mem, 64, mem_size);

    if (err == 0) {
      DB_DEBUG("db_alloc: Alloc'd %zu bytes at %p success. Reset all blocks...",
               (size_t)mem_size, (void *)(*mem))
      explicit_bzero(*mem, mem_size);
    } else {
      DB_DEBUG("db_alloc: error %d from posix_memalign", err)
      *mem = NULL;
    }

    return err;
  }

  DB_DEBUG("db_alloc: Unable to alloc. Invalid pointer or already initialized")
  return DB_UNABLE_TO_ALLOCATE_USER_MEMORY;
}

inline void db_free(void **mem) {
  DB_DEBUG("Entering db_free ...")
  if ((mem != NULL) && (*mem != NULL)) {
    DB_DEBUG("Destroying %p ...", (void *)*mem)
    free(*mem);
    *mem = NULL;
    DB_DEBUG("Destroyed")
  }
}

inline void db_clear_and_free(void **mem, size_t mem_size) {
  DB_DEBUG("Entering db_clear_and_free ...")
  if ((mem != NULL) && (*mem != NULL)) {
    DB_DEBUG("Destroying %p of size %zu bytes...", (void *)*mem, mem_size)
    if (mem_size) explicit_bzero(*mem, mem_size);

    free(*mem);
    *mem = NULL;
    DB_DEBUG("Destroyed")
  }
}
