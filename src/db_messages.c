#include <stdarg.h>
#include <db_service.h>
#include <db_config.h>
#include <string.h>

void set_db_service_error(DB_SERVICE *db_service, int error, char *fmt, ...)
{
  va_list args;

  if (db_service->err)
    return; // Error already exists

  db_service->err = error;

  int len;

  va_start(args, fmt);
  len = vsnprintf(db_service->message, DB_MESSAGE_LEN, fmt, args);
  va_end(args);

#define UNABLE_SET_MESSAGE_ERROR "Unable to set message"
  if (len >=0)
    db_service->message_len = (size_t)len;
  else {
    strncpy(db_service->message, UNABLE_SET_MESSAGE_ERROR, sizeof(db_service->message));
    db_service->message_len = (sizeof(UNABLE_SET_MESSAGE_ERROR) - 1);
    return;
  }
#undef UNABLE_SET_MESSAGE_ERROR
}
