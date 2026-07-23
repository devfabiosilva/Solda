#ifndef DB_H
 #define DB_H

#include <stdint.h>
#include <time.h>
#include <stdbool.h>
#include <db_config.h>

#define DB_SET_TOUCHED(x) (1<<x)

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
  SERVICE_REQUEST_NEW_AND_DELETED_BEFORE_SAVE,
  SERVICE_REQUEST_READ_FROM_DATABASE
} SERVICE_REQUEST_FLAG;

typedef struct service_request_t {
  uint32_t id;                                     // PK
  int32_t version;
  uint32_t repair_request_id;                      // FK for request
  int32_t touched;
  time_t created_at;                               // Created service request
  SERVICE_REQUEST_FLAG flag;                       // Only on edit/update mode. This field will NOT record at database
  int32_t quantity;                                // Quantity > 0
  MONETARY_TYPE monetary_type;                     // Monetary type
  int64_t unity_price;                             // Unity price (required)
  char description[LONG_DESCRIPTION_BUF];          // Description (required)
} SERVICE;

typedef enum service_request_touched_e {
  SERVICE_TOUCHED = true,
  SERVICE_VERSION_TOUCHED = DB_SET_TOUCHED(1),
  SERVICE_REPAIR_REQUEST_ID = DB_SET_TOUCHED(2),
  SERVICE_CREATED_AT = DB_SET_TOUCHED(3),
  SERVICE_QUANTITY = DB_SET_TOUCHED(4),
  SERVICE_MONETARY_TYPE = DB_SET_TOUCHED(5),
  SERVICE_UNITY_PRICE = DB_SET_TOUCHED(6),
  SERVICE_DESCRIPTION = DB_SET_TOUCHED(7)
} SERVICE_REQUEST_TOUCHED;

typedef struct service_requests_t {
  size_t n;                                        // Actual array size
  size_t array_max_len;                            // Alloc'd array size
  SERVICE *array;                                   // Alloc'd pointer for array. Recyclable. Must be free
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
  REPAIR_REQUEST_NEW_AND_DELETED_BEFORE_SAVE,
  REPAIR_REQUEST_READ_FROM_DATABASE
} REPAIR_REQUEST_FLAG;

typedef struct repair_request_t {
  int32_t id;                                         // PK repair request
  int32_t client_id;                                  // FK For client id
  int32_t touched;                                    // is touched?
  int32_t version;
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
  int64_t labor_budget;                               // labor_bugdet // Fixed point. TODO check validation
  time_t delivery_date;                               // Delivery date
  time_t warranty;                                    // Total day from delivery date
  SERVICE_REQUESTS optional_service_requests;         // ARRAY NULLABLE List of service requests
} REPAIR;

typedef enum repair_touched_e {
  REPAIR_COMMAND_TOUCHED = true,
  REPAIR_CLIENT_ID_COMMAND_TOUCHED = DB_SET_TOUCHED(1),
  REPAIR_VERSION_TOUCHED = DB_SET_TOUCHED(2),
  REPAIR_STATUS_COMMAND_TOUCHED = DB_SET_TOUCHED(3),
  REPAIR_IS_BUDGET_COMMAND_TOUCHED = DB_SET_TOUCHED(4),
  REPAIR_DEVICE_PROBLEM_COMMAND_TOUCHED = DB_SET_TOUCHED(5),
  REPAIR_BRAND_MODEL_COMMAND_TOUCHED = DB_SET_TOUCHED(6),
  REPAIR_SERIAL_MODEL_COMMAND_TOUCHED = DB_SET_TOUCHED(7),
  REPAIR_CLAIMED_DEFECT_COMMAND_TOUCHED = DB_SET_TOUCHED(8),
  REPAIR_OBSERVATIONS_COMMAND_TOUCHED = DB_SET_TOUCHED(9),
  REPAIR_MONETARY_TYPE_COMMAND_TOUCHED = DB_SET_TOUCHED(10),
  REPAIR_EXPECTED_BUDGET_DATE_COMMAND_TOUCHED = DB_SET_TOUCHED(11),
  REPAIR_EXPECTED_DELIVERY_DATE_DATE_COMMAND_TOUCHED = DB_SET_TOUCHED(12),
  REPAIR_LABOR_BUDEGET_DATE_COMMAND_TOUCHED = DB_SET_TOUCHED(13),
  REPAIR_DELIVERY_DATE_COMMAND_TOUCHED = DB_SET_TOUCHED(14),
  REPAIR_WARRANTY_DATE_COMMAND_TOUCHED = DB_SET_TOUCHED(15)
} REPAIR_TOUCHED;

typedef struct repair_requests_t {
  size_t n;                                        // Actual array size
  size_t array_max_len;                            // Alloc'd array size
  REPAIR *array;                                   // Alloc'd pointer for array. Recyclable. Must be free
} REPAIR_REQUESTS;

// END DEVICE DATA

// Only on edit/update mode. This field will NOT record at database
typedef enum client_data_flag_e {
  CLIENT_DATA_INIT = 0,
  CLIENT_DATA_NEW,
  CLIENT_DATA_UPDATE,
  CLIENT_DATA_DELETE,
  CLIENT_DATA_NEW_AND_DELETED_BEFORE_SAVE,
  CLIENT_DATA_READ_FROM_DATABASE
} CLIENT_DATA_FLAG;

// BEGIN CLIENT USER TABLE
typedef struct client_data_t {
  int32_t touched;                                  // For edit/add/update only flag: true if is used (read to flush in database)
  int32_t id;                                       // PK (required). TODO check sizeof Postgres INTEGER
  int32_t technician_id;                            // FK Technician id
  int32_t version;                                  //
  time_t created_at;                                // Created user timestamp for Solda client user
  CLIENT_DATA_FLAG flag;                            // Flag. This will not be flushed in database
  char cpf[CPF_BUF];                                // CPF (required) - UNIQUE
  char name[NAME_BUF];                              // Client name (required)
  char address[ADDRESS_BUF];                        // Client address
  char district_city[ADDRESS_BUF];                  // District and city
  char email[EMAIL_ADDRESS_BUF];                    // (Required) email address;
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

// Only on edit/update mode. This field will NOT record at database
typedef enum technician_data_flag_e {
  TECHNICIAN_DATA_INIT = 0,
  TECHNICIAN_DATA_NEW,
  TECHNICIAN_DATA_UPDATE,
  TECHNICIAN_DATA_DELETE,
  TECHNICIAN_READ_FROM_DATABASE
} TECHNICIAN_DATA_FLAG;

_Static_assert(true == DB_SET_TOUCHED(0), "true value must be equal 1");
typedef enum technician_data_touched_e {
  TECHNICIAN_TOUCHED = true,
  TECHNICIAN_VERSION_TOUCHED = DB_SET_TOUCHED(1),
  TECHNICIAN_RULES_TOUCHED = DB_SET_TOUCHED(2),
  TECHNICIAN_NAME_TOUCHED = DB_SET_TOUCHED(3),
  TECHNICIAN_EMAIL_TOUCHED = DB_SET_TOUCHED(4),
  TECHNICIAN_PHONE_NUMBER_TOUCHED = DB_SET_TOUCHED(5),
  TECHNICIAN_ID_TOUCHED = DB_SET_TOUCHED(6),
  TECHNICIAN_CREATED_AT_TOUCHED = DB_SET_TOUCHED(7)
} TECHNICIAN_DATA_TOUCHED;

typedef enum client_data_touched_e {
  CLIENT_TOUCHED = true,
  CLIENT_TECHNICIAN_ID_COMMAND_TOUCHED = DB_SET_TOUCHED(1),
  CLIENT_VERSION_TOUCHED = DB_SET_TOUCHED(2),
  CLIENT_CPF_COMMAND_TOUCHED = DB_SET_TOUCHED(3),
  CLIENT_NAME_COMMAND_TOUCHED = DB_SET_TOUCHED(4),
  CLIENT_ADDRESS_COMMAND_TOUCHED = DB_SET_TOUCHED(5),
  CLIENT_DISTRICT_CITY_COMMAND_TOUCHED = DB_SET_TOUCHED(6),
  CLIENT_EMAIL_COMMAND_TOUCHED = DB_SET_TOUCHED(7),
  CLIENT_PHONE_NUMBER_COMMAND_TOUCHED = DB_SET_TOUCHED(8),
  CLIENT_ID_COMMAND_TOUCHED = DB_SET_TOUCHED(9),
  CLIENT_VERSION_COMMAND_TOUCHED = DB_SET_TOUCHED(10),
  CLIENT_CREATED_AT_COMMAND_TOUCHED = DB_SET_TOUCHED(11)
} CLIENT_DATA_TOUCHED;

typedef struct technician_data_t{
  int32_t touched;
  int32_t id;
  int32_t version;
  TECHNICIAN_RULES rules;
  TECHNICIAN_DATA_FLAG flag;
  time_t created_at;
  char name[SHORT_NAME_BUF];
  char email[EMAIL_ADDRESS_BUF];
  char phone_number[PHONE_BUF];                     // (Required) Phone number;
  CLIENT_DATA_REQUESTS client_requests;
} TECHNICIAN_DATA;

typedef struct technician_data_requests_t {
  size_t n;                                         // Actual array size
  size_t array_max_len;                             // Alloc'd array size
  TECHNICIAN_DATA *array;                           // Alloc'd pointer for array. Recyclable. Must be free
} TECHNICIAN_DATA_REQUESTS;

//END TECHNICIAN TABLE

// BEGIN SERVICE MANIPULATION
void service_requests_clear(SERVICE_REQUESTS *);
void service_request_clear(SERVICE *);
// END SERVICE MANIPULATION

void repair_request_clear(REPAIR *);
void repair_requests_clear(REPAIR_REQUESTS *);

// END REFACTORING
void client_data_clear(CLIENT_DATA *);
// BEGIN TECHNICIAN
int technician_data_requests_init(TECHNICIAN_DATA_REQUESTS **);
void technician_data_requests_free(TECHNICIAN_DATA_REQUESTS **);

int technician_acquire_technician_data_from_array(size_t *, TECHNICIAN_DATA **, TECHNICIAN_DATA_REQUESTS *);
void technician_data_clear(TECHNICIAN_DATA *);
void technician_data_requests_clear(TECHNICIAN_DATA_REQUESTS *);

int technician_acquire_client_data_from_array(size_t *, CLIENT_DATA **, size_t, TECHNICIAN_DATA_REQUESTS *);
int technician_acquire_client_data_requests_from_array(CLIENT_DATA_REQUESTS **, size_t, TECHNICIAN_DATA_REQUESTS *);
int technician_acquire_repair_requests_from_array(REPAIR_REQUESTS **, size_t, size_t, TECHNICIAN_DATA_REQUESTS *);
int technician_acquire_repair_request_from_array(size_t *, REPAIR **, size_t, size_t, TECHNICIAN_DATA_REQUESTS *);
int technician_acquire_service_requests_from_array(SERVICE_REQUESTS **, size_t, size_t, size_t, TECHNICIAN_DATA_REQUESTS *);
int technician_acquire_service_request_from_array(size_t *, SERVICE **, size_t, size_t, size_t, TECHNICIAN_DATA_REQUESTS *);

// END TECHNICIAN
#endif
