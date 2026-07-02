#ifndef DB_COMMAND_MACROS_H
 #define DB_COMMAND_MACROS_H

#define DB_COPY_STR_FROM(dst, src) \
    if (src) {\
        strncpy(dst, src, sizeof(dst)-1); \
    }

#endif