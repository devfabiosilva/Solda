#ifndef DB_SERVICE_H
 #define DB_SERVICE_H

#include <postgresql/libpq-fe.h>
#include <db.h>
#include <db_config.h>

typedef struct db_service_t {
  int err;                                              // Error status
  PGconn *conn;                                         // Postgres connection
  TECHNICIAN_DATA_REQUESTS *technician_data_requests;   // Manipulators for technician, clients, repair and services
  char message[DB_MESSAGE_LEN + 1];
} DB_SERVICE;

int db_service_init(DB_SERVICE **, char *);
void db_service_free(DB_SERVICE **);

#endif

