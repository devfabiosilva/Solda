#ifndef DB_CONFIG_H
 #define DB_CONFIG_H

#define MIN_REPAIR_REQUEST_INITIAL 8 // MIN request list element per client at init
#define MAX_REPAIR_REQUESTS_LIMIT 512 // MAX request list element per client

_Static_assert(MIN_REPAIR_REQUEST_INITIAL > 0, "MIN_REPAIR_REQUEST_INITIAL must be greater than 0");
_Static_assert(((MIN_REPAIR_REQUEST_INITIAL & 1) == 0) && ((MAX_REPAIR_REQUESTS_LIMIT & 1) == 0), "MIN_REPAIR_REQUEST_INITIAL and MAX_REPAIR_REQUESTS_LIMIT must be multiple of 2");
_Static_assert(MAX_REPAIR_REQUESTS_LIMIT > MIN_REPAIR_REQUEST_INITIAL, "Wrong REPAIR REQUEST bound limit");

#endif
