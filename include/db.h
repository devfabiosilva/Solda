#ifndef DB_H
 #define DB_H

#include <stdint.h>
#include <time.h>
#include <stdbool.h>

// BEGIN DEVICE DATA
#define SET_DEVICE_DATA(x) (int)(1<<x)
typedef enum solda_electronic_device_data_e {
  DEVICE_DOES_NOT_TURN_ON     =  SET_DEVICE_DATA(0),
  BROKEN_SCREEN               =  SET_DEVICE_DATA(1),
  TOUCH_DOES_NOT_WORKS        =  SET_DEVICE_DATA(2),
  NO_BUTTONS                  =  SET_DEVICE_DATA(3),
  SWOLLEN_BATTERY             =  SET_DEVICE_DATA(4),
  IN_LOOP                     =  SET_DEVICE_DATA(5),
  NO_CHIP                     =  SET_DEVICE_DATA(6),
  DEFECTIVE_MICROPHONE        =  SET_DEVICE_DATA(7),
  DEVICE_DOES_NOT_CHARGE      =  SET_DEVICE_DATA(8),
  BROKEN_CAMERA               =  SET_DEVICE_DATA(9),
  WITHOUT_INVOICE             =  SET_DEVICE_DATA(10),
  NO_AUDIO                    =  SET_DEVICE_DATA(11),
  INPUT_DEFECTIVE_MICROPHONE  =  SET_DEVICE_DATA(12),
  NO_LID                      =  SET_DEVICE_DATA(13)
} ELECTRONIC_DEVICE_DATA_ENUMERATOR;
#undef SET_DEVICE_DATA

typedef enum repair_request_status_e {
  REPAIR_REQUEST_STATUS_RECEIVED = 0,
  REPAIR_REQUEST_STATUS_DIAGNOSING,
  REPAIR_REQUEST_STATUS_REPAIRING,
  REPAIR_REQUEST_STATUS_READY,
  REPAIR_REQUEST_STATUS_DELIVERED
} REPAIR_REQUEST_STATUS;

// Only on edit/update mode. This field will NOT record at database
typedef enum repair_request_flag_e {
  REPAIR_REQUEST_NEW = 0,
  REPAIR_REQUEST_UPDATE,
  REPAIR_REQUEST_DELETE
} REPAIR_REQUEST_FLAG;

typedef struct solda_repair_requests_t {
  uint32_t id;                                    // PK repair request
  uint32_t client_id;                             // FK For client id
  REPAIR_REQUEST_STATUS status;                   // Request status
  bool is_bugdet;                                 // Binary: Bugdet or Work order
  REPAIR_REQUEST_FLAG flag;                       // Only on edit/update mode. This field will NOT record at database
  ELECTRONIC_DEVICE_DATA_ENUMERATOR device_data;  // Problem data
  char brand_model[64];                           // Brand or model
  char serial_number[64];                         // IMEI, serial number ...
  char claimed_defect[196];                       // Defect according to client user
  char observations[196];                         // Obs
  time_t expected_budget_date;                    // Expected bugdget date
  time_t expected_delivery_date;                  // Expected delivery date
  int64_t total_price;                            // Total price // Fixed point. TODO check validation
  time_t delivery_date;                           // Delivery date
  time_t warranty;                                // Total day from delivery date
} REPAIR_REQUEST;

typedef struct repair_request_t {
  size_t n;                                        // Actual array size
  size_t array_max_len;                            // Alloc'd array size
  REPAIR_REQUEST *array;                           // Alloc'd pointer for array. Recyclable. Must be free
} REPAIR_REQUESTS;

// END DEVICE DATA

// BEGIN CLIENT USER TABLE
typedef struct solda_client_data_t {
  bool touched;                                     // For edit/add/update only flag: true if is used (read to flush in database)
  time_t timestamp;                                 // Created user timestamp for Solda client user
  uint32_t id;                                      // PK (required). TODO check sizeof Postgres INTEGER
  char cpf[32];                                     // CPF (required) - UNIQUE
  char name[64];                                    // Client name (required)
  char address[128];                                // Client address
  char district_city[64];                           // District and city
  char email_address[32];                           // (Required) email address;
  char phone_number[16];                            // (Required) Phone number;
  REPAIR_REQUESTS repair_requests;                  // NOT NULL repair requests
} CLIENT_DATA;
// END CLIENT USER TABLE

// BEGIN CLIENT_DATA CONSTRUCTORS AND DESTRUCTORS
int client_data_init(CLIENT_DATA **);
void client_data_free(CLIENT_DATA **);
// END CLIENT_DATA CONSTRUCTORS AND DESTRUCTORS

// BEGIN CLIENT_DATA MANIPULATION
int client_data_clear(CLIENT_DATA *);
int client_add_repair_array(CLIENT_DATA *, REPAIR_REQUEST *, size_t);
// END CLIENT_DATA MANIPULATION

#endif
