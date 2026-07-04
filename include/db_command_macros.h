#ifndef DB_COMMAND_MACROS_H
 #define DB_COMMAND_MACROS_H

#define DB_COPY_STR_FROM(dst, src) \
    if (src) {\
        strncpy(dst, src, sizeof(dst)-1); \
    }

#define TECHNICIAN_MANIPULATE_HELPER(func, action, text) \
    if (technician_data) { \
        TECHNICIAN_DATA_FLAG flag = technician_data->flag; \
        TECHNICIAN_DATA_TOUCHED touched = 0; \
\
        text \
\
        va_list args; \
\
        va_start(args, technician_data); \
        DB_ADD_ENUM_COMMAND command; \
        void *p; \
        int err = 0; \
        while ((command = (DB_##action##_ENUM_COMMAND)va_arg(args, DB_ADD_ENUM_COMMAND))) { \
            p = (void *)va_arg(args, void *); \
            switch (command) { \
                case DB_TECHNICIAN_##action##_NAME_COMMAND: \
                    DB_COPY_STR_FROM(technician_data->name, (char *)p); \
                    touched |= TECHNICIAN_NAME_TOUCHED; \
                    break; \
                case DB_TECHNICIAN_##action##_EMAIL_COMMAND: \
                    DB_COPY_STR_FROM(technician_data->email, (char *)p) \
                    touched |= TECHNICIAN_EMAIL_TOUCHED; \
                    break; \
                case DB_TECHNICIAN_##action##_PHONE_NUMBER_COMMAND: \
                    DB_COPY_STR_FROM(technician_data->phone_number, (char *)p) \
                    touched |= TECHNICIAN_PHONE_NUMBER_TOUCHED; \
                    break; \
                case DB_TECHNICIAN_##action##_RULES: \
                    technician_data->rules = (TECHNICIAN_RULES)((intptr_t)p); \
                    touched |= TECHNICIAN_RULES_TOUCHED; \
                    break; \
                default: \
                    err = DB_TECHNICIAN_##action##_INVALID_COMMAND; \
                    goto func##_exit1; \
            } \
        } \
func##_exit1: \
        va_end(args); \
\
        if (err == 0) { \
            technician_data->touched = touched; \
            technician_data->flag = flag; \
        } \
        return 0; \
    } \
\
    return DB_TECHNICIAN_DATA_##action##_INVALID;

#endif

