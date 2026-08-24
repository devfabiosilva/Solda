#include <db_log.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#define DB_LOG_RESET "\033[0m ]"
#define DB_ERROR_CODE "[ \e[31;1m ERROR" DB_LOG_RESET
#define DB_INFO_CODE "[ \e[32;1m INFO " DB_LOG_RESET
#define DB_WARNING_CODE "[ \e[33;1m WARN " DB_LOG_RESET
#define DB_DEBUG_CODE "[ \e[1;3m DEBUG" DB_LOG_RESET

#ifdef SOLDA_DEBUG
void _db_log_error(const char *func, const char *file, int line,
                   const char *fmt, ...) {
  int len;
  FILE *fd = stdout;
  va_list args;

  fprintf(fd, " %lu - %s Function %s at file %s in line: %d - ",
          (unsigned long)time(NULL), DB_ERROR_CODE, func, file, line);

  va_start(args, fmt);
  len = vfprintf(fd, fmt, args);
  va_end(args);

  if (len >= 0)
    fprintf(fd, "\n");
  else
    fprintf(stderr, "ERROR DEBUG: Unable to log. Failed");
}
#endif

void _db_log(DB_LOG log_type, const char *fmt, ...) {
  int len;
  FILE *fd = stdout;
  va_list args;
  const char *code;

  switch (log_type) {
    case LOG_INFO:
      code = DB_INFO_CODE;
      break;
    case LOG_ERROR:
      fd = stderr;
      code = DB_ERROR_CODE;
      break;
    case LOG_WARN:
      code = DB_WARNING_CODE;
      break;
    default:
      code = DB_DEBUG_CODE;
  }

  fprintf(fd, " %lu - %s ", (unsigned long)time(NULL), code);

  va_start(args, fmt);
  len = vfprintf(fd, fmt, args);
  va_end(args);

  if (len >= 0)
    fprintf(fd, "\n");
  else
    fprintf(stderr, "Unable to log. Failed");
}
