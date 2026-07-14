#ifndef DB_SERVICE_H
 #define DB_SERVICE_H

#include <postgresql/libpq-fe.h>
#include <db.h>
#include <db_config.h>
#include <stdalign.h>

typedef struct db_service_t {
  PGconn *conn;                                         // Postgres connection
  TECHNICIAN_DATA_REQUESTS *technician_data_requests;   // Manipulators for technician, clients, repair and services
  TECHNICIAN_DATA *technician_data_list;                // Technician data list buffer. Array must be free
  CLIENT_DATA *client_data_list;                        // Client data list. Array must be free
  REPAIR *repair_list;                                  // Repair data list. Array must be free
  SERVICE *service_list;                                // Service data list. Array must be free
  size_t message_len;
  char message[DB_MESSAGE_LEN + 1];
  int err;                                              // Error status
} DB_SERVICE;

int db_service_init(DB_SERVICE **, char *);
void db_service_free(DB_SERVICE **);

int db_service_load_technicians(DB_SERVICE *, uint32_t, uint32_t);

#define DB_SERVICE_RETURN return db_service->err;

#define DB_MESSAGE(p) p->message

#endif

