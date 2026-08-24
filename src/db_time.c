#include <db_log.h>
#include <time.h>

const char *db_time(char *buf, size_t buf_sz, time_t *time) {
  strftime(buf, buf_sz, "%Y-%m-%d %H:%M:%S UTC", gmtime(time));
  return buf;
}
