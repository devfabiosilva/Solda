#ifndef DB_H
 #define DB_H

#include <stdint.h>
#include <time.h>
#include <stdbool.h>
#include <db_config.h>

typedef enum monetary_type_e {
  BRL = 0,
  US_DOLLAR,
  BITCOIN
} MONETARY_TYPE;

// BEGIN SERVICE REQUEST

// Only on edit/update mode. This field will NOT record at database
typedef enum service_request_flag_e {
  SERVICE_REQUEST_INIT = 0,
  SERVICE_REQUEST_NEW,
  SERVICE_REQUEST_UPDATE,
  SERVICE_REQUEST_DELETE,
  SERVICE_REQUEST_NEW_AND_DELETED_BEFORE_SAVE
} SERVICE_REQUEST_FLAG;

typedef struct service_request_t {
  uint32_t id;                                     // PK
  uint32_t repair_request_id;                      // FK for request
  time_t created_at;                               // Created service request
  SERVICE_REQUEST_FLAG flag;                       // Only on edit/update mode. This field will NOT record at database
  int32_t quantity;                                // Quantity > 0
  MONETARY_TYPE monetary_type;                     // Monetary type
  int64_t unity_price;                             // Unity price (required)
  char description[LONG_DESCRIPTION_BUF];          // Description (required)
} SERVICE_REQUEST;

typedef struct service_requests_t {
  size_t n;                                        // Actual array size
  size_t array_max_len;                            // Alloc'd array size
  SERVICE_REQUEST *array;                           // Alloc'd pointer for array. Recyclable. Must be free
} SERVICE_REQUESTS;

// END SERVICE REQUEST

// BEGIN DEVICE DATA
#define SET_DEVICE_DATA(x) (int)(1<<x)
typedef enum solda_electronic_device_problem_e {
  NO_CHOICE                   = 0,
  DEVICE_DOES_NOT_TURN_ON     = SET_DEVICE_DATA(0),
  BROKEN_SCREEN               = SET_DEVICE_DATA(1),
  TOUCH_DOES_NOT_WORKS        = SET_DEVICE_DATA(2),
  NO_BUTTONS                  = SET_DEVICE_DATA(3),
  SWOLLEN_BATTERY             = SET_DEVICE_DATA(4),
  IN_LOOP                     = SET_DEVICE_DATA(5),
  NO_CHIP                     = SET_DEVICE_DATA(6),
  DEFECTIVE_MICROPHONE        = SET_DEVICE_DATA(7),
  DEVICE_DOES_NOT_CHARGE      = SET_DEVICE_DATA(8),
  BROKEN_CAMERA               = SET_DEVICE_DATA(9),
  WITHOUT_INVOICE             = SET_DEVICE_DATA(10),
  NO_AUDIO                    = SET_DEVICE_DATA(11),
  INPUT_DEFECTIVE_MICROPHONE  = SET_DEVICE_DATA(12),
  NO_LID                      = SET_DEVICE_DATA(13)
} ELECTRONIC_DEVICE_PROBLEM;
#undef SET_DEVICE_DATA

typedef enum repair_request_status_e {
  REPAIR_REQUEST_STATUS_NONE = 0,
  REPAIR_REQUEST_STATUS_RECEIVED,
  REPAIR_REQUEST_STATUS_DIAGNOSING,
  REPAIR_REQUEST_STATUS_REPAIRING,
  REPAIR_REQUEST_STATUS_READY,
  REPAIR_REQUEST_STATUS_DELIVERED
} REPAIR_REQUEST_STATUS;

// Only on edit/update mode. This field will NOT record at database
typedef enum repair_request_flag_e {
  REPAIR_REQUEST_INIT = 0,
  REPAIR_REQUEST_NEW,
  REPAIR_REQUEST_UPDATE,
  REPAIR_REQUEST_DELETE,
  REPAIR_REQUEST_NEW_AND_DELETED_BEFORE_SAVE
} REPAIR_REQUEST_FLAG;

typedef struct repair_request_t {
  int32_t id;                                         // PK repair request
  int32_t client_id;                                  // FK For client id
  time_t created_at;                                  // Created request timestamp for Solda client user
  REPAIR_REQUEST_STATUS status;                       // Request status
  bool is_bugdet;                                     // Binary: Bugdet or Work order
  REPAIR_REQUEST_FLAG flag;                           // Only on edit/update mode. This field will NOT record at database
  ELECTRONIC_DEVICE_PROBLEM device_problem;           // Problem data
  char brand_model[SHORT_NAME_BUF];                   // Brand or model
  char serial_number[SHORT_NAME_BUF];                 // IMEI, serial number ...
  char claimed_defect[LONG_DESCRIPTION_BUF];          // Defect according to client user
  char observations[LONG_DESCRIPTION_BUF];            // Obs
  MONETARY_TYPE monetary_type;                        // Monetary type
  time_t expected_budget_date;                        // Expected bugdget date
  time_t expected_delivery_date;                      // Expected delivery date
  int64_t labor_bugdet;                               // labor_bugdet // Fixed point. TODO check validation
  time_t delivery_date;                               // Delivery date
  time_t warranty;                                    // Total day from delivery date
  SERVICE_REQUESTS optional_service_requests;         // ARRAY NULLABLE List of service requests
} REPAIR_REQUEST;

typedef struct repair_requests_t {
  size_t n;                                        // Actual array size
  size_t array_max_len;                            // Alloc'd array size
  REPAIR_REQUEST *array;                           // Alloc'd pointer for array. Recyclable. Must be free
} REPAIR_REQUESTS;

// END DEVICE DATA

// BEGIN CLIENT USER TABLE
typedef struct client_data_t {
  bool touched;                                     // For edit/add/update only flag: true if is used (read to flush in database)
  int32_t id;                                       // PK (required). TODO check sizeof Postgres INTEGER
  int32_t technician_id;                            // FK Technician id
  time_t created_at;                                // Created user timestamp for Solda client user
  char cpf[CPF_BUF];                                // CPF (required) - UNIQUE
  char name[NAME_SZ];                               // Client name (required)
  char address[ADDRESS_BUF];                        // Client address
  char district_city[ADDRESS_BUF];                  // District and city
  char email[EMAIL_ADDRESS_SZ];                     // (Required) email address;
  char phone_number[PHONE_BUF];                     // (Required) Phone number;
  REPAIR_REQUESTS repair_requests;                  // NOT NULL repair requests
} CLIENT_DATA;

typedef struct client_data_list_t {
  size_t n;                                         // Actual array size
  size_t array_max_len;                             // Alloc'd array size
  CLIENT_DATA *array;                               // Alloc'd pointer for array. Recyclable. Must be free
} CLIENT_DATA_REQUESTS;

// END CLIENT USER TABLE

//BEGIN TECHNICIAN TABLE
#define SET_TECHNICIAN_RULES(x) (int)(1<<x)
typedef enum technician_rules_e {
  NO_ACCESS                       = 0,                        // NO DB ACCESS
  IS_ROOT_ADMIN_SUPREME           = SET_TECHNICIAN_RULES(0),  // Supreme root admin. Can delete|update|read DB, clients others admins, and technicians
  IS_ADMIN                        = SET_TECHNICIAN_RULES(1),  // Admin. Can Can delete|update|read DB, clients and technicians
  CAN_DELETE_CLIENT               = SET_TECHNICIAN_RULES(2),  // Regular user NOT (IS_ROOT_ADMIN_SUPREME|IS_ADMIN) can delete user if enabled
  CAN_EDIT_CLIENT                 = SET_TECHNICIAN_RULES(3),  // Regular user NOT (IS_ROOT_ADMIN_SUPREME|IS_ADMIN) can update user if enabled
  CAN_READ_CLIENT                 = SET_TECHNICIAN_RULES(4),  // Regular user NOT (IS_ROOT_ADMIN_SUPREME|IS_ADMIN) can read user if enabled
  CAN_CHANGE_TECHNICIAN_IN_CLIENT = SET_TECHNICIAN_RULES(5)   // Regular user NOT (IS_ROOT_ADMIN_SUPREME|IS_ADMIN) can change technician in user if enabled
} TECHNICIAN_RULES;
#undef SET_TECHNICIAN_RULES

typedef struct technician_data_t{
  bool touched;
  int32_t id;
  TECHNICIAN_RULES rules;
  time_t created_at;
  char name[SHORT_NAME_SZ];
  char email[EMAIL_ADDRESS_SZ];
  CLIENT_DATA_REQUESTS client_requests;
} TECHNICIAN_DATA;

typedef struct technician_data_requests_t {
  size_t n;                                         // Actual array size
  size_t array_max_len;                             // Alloc'd array size
  TECHNICIAN_DATA *array;                           // Alloc'd pointer for array. Recyclable. Must be free
} TECHNICIAN_DATA_REQUESTS;

//END TECHNICIAN TABLE

// BEGIN REPAIR MANIPULATION
int repair_acquire_service(size_t *, SERVICE_REQUEST **, REPAIR_REQUEST *);
// END REPAIR MANIPULATION

// BEGIN CLIENT_DATA MANIPULATION
//void client_data_clear(CLIENT_DATA *);
// END CLIENT_DATA MANIPULATION

// BEGIN REPAIR MANIPULATION
void repair_request_clear(REPAIR_REQUEST *);
void repair_requests_clear(REPAIR_REQUESTS *);
int client_acquire_repair(size_t *, REPAIR_REQUEST **, CLIENT_DATA *);
// END REPAIR MANIPULATION

// BEGIN SERVICE MANIPULATION
void service_requests_clear(SERVICE_REQUESTS *);
// END SERVICE MANIPULATION

// BEGIN REFACTORING CLIENT REQUESTS
// TODO implement prototype data
int client_acquire_client_data_from_array(size_t *, CLIENT_DATA **, CLIENT_DATA_REQUESTS *);
int client_aquire_repair_request_data(size_t *, REPAIR_REQUEST **, CLIENT_DATA *);
int client_aquire_repair_data_requests(REPAIR_REQUESTS **, CLIENT_DATA *);
void client_data_clear(CLIENT_DATA *);
void client_data_requests_clear(CLIENT_DATA_REQUESTS *);
// END REFACTORING

// BEGIN TECHNICIAN
int technician_data_requests_init(TECHNICIAN_DATA_REQUESTS **);
void technician_data_requests_free(TECHNICIAN_DATA_REQUESTS **);
int technician_acquire_technician_data_from_array(size_t *, TECHNICIAN_DATA **, TECHNICIAN_DATA_REQUESTS *);
int technician_acquire_client_data(size_t *, CLIENT_DATA **, TECHNICIAN_DATA *);
int technician_acquire_client_data_requests(CLIENT_DATA_REQUESTS **, TECHNICIAN_DATA *);
void technician_data_clear(TECHNICIAN_DATA *);
void technician_data_requests_clear(TECHNICIAN_DATA_REQUESTS *);
// END TECHNICIAN
#endif
