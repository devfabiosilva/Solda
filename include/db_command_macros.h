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
        DB_##action##_ENUM_COMMAND command; \
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

#define CLIENT_DATA_MANIPULATE_HELPER(func, action, text) \
    if (client_data) { \
        CLIENT_DATA_FLAG flag = client_data->flag; \
\
        text \
\
        int32_t touched = 0; \
        va_list args; \
\
        va_start(args, client_data); \
        DB_##action##_ENUM_COMMAND command; \
        void *p; \
        int err = 0; \
        while ((command = (DB_##action##_ENUM_COMMAND)va_arg(args, DB_##action##_ENUM_COMMAND))) { \
            p = (void *)va_arg(args, void *); \
            switch (command) { \
                case DB_CLIENT_##action##_TECHNICIAN_ID_COMMAND: \
                    size_t id_as_size_t = (size_t)((intptr_t)p); \
                    if (id_as_size_t <= INT32_MAX) { \
                        client_data->technician_id = (int32_t)id_as_size_t; \
                        touched |= CLIENT_TECHNICIAN_ID_COMMAND_TOUCHED; \
                        break; \
                    } \
                    err = DB_CLIENT_##action##_TECHNICIAN_ID_LIMIT_REACHED; \
                    goto func##_exit1; \
                case DB_CLIENT_##action##_CPF_COMMAND: \
                    DB_COPY_STR_FROM(client_data->cpf, (char *)p) \
                    touched |= CLIENT_CPF_COMMAND_TOUCHED; \
                    break; \
                case DB_CLIENT_##action##_NAME_COMMAND: \
                    DB_COPY_STR_FROM(client_data->name, (char *)p) \
                    touched |= CLIENT_NAME_COMMAND_TOUCHED; \
                    break; \
                case DB_CLIENT_##action##_ADDRESS_COMMAND: \
                    DB_COPY_STR_FROM(client_data->address, (char *)p) \
                    touched |= CLIENT_ADDRESS_COMMAND_TOUCHED; \
                    break; \
                case DB_CLIENT_##action##_DISTRICT_CITY_COMMAND: \
                    DB_COPY_STR_FROM(client_data->district_city, (char *)p) \
                    touched |= CLIENT_DISTRICT_CITY_COMMAND_TOUCHED; \
                    break; \
                case DB_CLIENT_##action##_EMAIL_COMMAND: \
                    DB_COPY_STR_FROM(client_data->email, (char *)p) \
                    touched |= CLIENT_EMAIL_COMMAND_TOUCHED; \
                    break; \
                case DB_CLIENT_##action##_PHONE_NUMBER_COMMAND: \
                    DB_COPY_STR_FROM(client_data->phone_number, (char *)p) \
                    touched |= CLIENT_PHONE_NUMBER_COMMAND_TOUCHED; \
                    break; \
                default: \
                    err = DB_CLIENT_##action##_INVALID_COMMAND; \
                    goto func##_exit1; \
            } \
        } \
\
func##_exit1: \
        va_end(args); \
\
        if (err == 0) { \
            client_data->touched = touched; \
            client_data->flag = flag; \
        } \
\
        return 0; \
    } \
\
    return DB_CLIENT_##action##_INVALID;

#endif

