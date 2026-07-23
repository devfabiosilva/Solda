#ifndef DB_COMMAND_MACROS_H
 #define DB_COMMAND_MACROS_H

#define DB_COPY_STR_FROM(dst, src) \
    if (src) {\
        strncpy(dst, src, sizeof(dst)-1); \
    } else { \
        dst[0] = 0; \
    }

#define TECHNICIAN_MANIPULATE_HELPER(func, action, text) \
    DB_DEBUG(#func ": Access technician_data at %p", technician_data) \
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
                case DB_TECHNICIAN_##action##_ID_COMMAND: \
                    ssize_t id_as_ssize_t = (ssize_t)((intptr_t)p); \
                    if ((id_as_ssize_t <= (ssize_t)INT32_MAX) && (id_as_ssize_t >= (ssize_t)INT32_MIN)) { \
                        technician_data->id = (int32_t)id_as_ssize_t; \
                        touched |= TECHNICIAN_ID_TOUCHED; \
                        DB_DEBUG("\ttechnician_data->id = %d", technician_data->id) \
                        break; \
                    } \
                    DB_DEBUG("\ttechnician_data->id limit reached DB_TECHNICIAN_" #action "_ID_LIMIT_REACHED(%d)", DB_TECHNICIAN_##action##_ID_LIMIT_REACHED) \
                    err = DB_TECHNICIAN_##action##_ID_LIMIT_REACHED; \
                    goto func##_exit1; \
                case DB_TECHNICIAN_##action##_NAME_COMMAND: \
                    DB_COPY_STR_FROM(technician_data->name, (char *)p); \
                    touched |= TECHNICIAN_NAME_TOUCHED; \
                    DB_DEBUG("\tAdded technician_data->name = %s", technician_data->name) \
                    break; \
                case DB_TECHNICIAN_##action##_CREATED_AT: \
                    technician_data->created_at = (time_t)((intptr_t)p); \
                    touched |= TECHNICIAN_CREATED_AT_TOUCHED; \
                    DB_DEBUG("\tAdded technician_data->created_at = %zu", (size_t)technician_data->created_at) \
                    break; \
                case DB_TECHNICIAN_##action##_VERSION: \
                    technician_data->version = (int32_t)((intptr_t)p); \
                    touched |= TECHNICIAN_VERSION_TOUCHED; \
                    DB_DEBUG("\tAdded technician_data->version = %d", technician_data->version) \
                    break; \
                case DB_TECHNICIAN_##action##_EMAIL_COMMAND: \
                    DB_COPY_STR_FROM(technician_data->email, (char *)p) \
                    touched |= TECHNICIAN_EMAIL_TOUCHED; \
                    DB_DEBUG("\ttechnician_data->email = %s", technician_data->email) \
                    break; \
                case DB_TECHNICIAN_##action##_PHONE_NUMBER_COMMAND: \
                    DB_COPY_STR_FROM(technician_data->phone_number, (char *)p) \
                    touched |= TECHNICIAN_PHONE_NUMBER_TOUCHED; \
                    DB_DEBUG("\ttechnician_data->phone_number = %s", technician_data->phone_number) \
                    break; \
                case DB_TECHNICIAN_##action##_RULES: \
                    technician_data->rules = (TECHNICIAN_RULES)((intptr_t)p); \
                    DB_DEBUG("\ttechnician_data->rules = %d", technician_data->rules) \
                    touched |= TECHNICIAN_RULES_TOUCHED; \
                    break; \
                default: \
                    err = DB_TECHNICIAN_##action##_INVALID_COMMAND; \
                    DB_DEBUG("Error. Invalid command %d with error %d", command, err) \
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
        DB_DEBUG(#func ": ending with status %d", err) \
        return err; \
    } \
\
    DB_DEBUG(#func ": ending with error DB_TECHNICIAN_DATA_" #action "_INVALID") \
    return DB_TECHNICIAN_DATA_##action##_INVALID;

#define CLIENT_DATA_MANIPULATE_HELPER(func, action, text) \
    DB_DEBUG(#func ": Access client data at %p", client_data) \
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
                case DB_CLIENT_##action##_VERSION_COMMAND: \
                    ssize_t version_as_ssize_t = (ssize_t)((intptr_t)p); \
                    if ((version_as_ssize_t <= (ssize_t)INT32_MAX) && (version_as_ssize_t >= (ssize_t)INT32_MIN)) { \
                        client_data->version = (int32_t)version_as_ssize_t; \
                        touched |= CLIENT_VERSION_COMMAND_TOUCHED; \
                        DB_DEBUG("\tclient_data->version = %d", client_data->version) \
                        break; \
                    } \
                    DB_DEBUG("\tclient_data->version limit reached DB_CLIENT_" #action "_VERSION_LIMIT_REACHED(%d)", DB_CLIENT_##action##_VERSION_LIMIT_REACHED) \
                    err = DB_CLIENT_##action##_VERSION_LIMIT_REACHED; \
                    goto func##_exit1; \                
                case DB_CLIENT_##action##_ID_COMMAND: \
                    ssize_t id_as_ssize_t = (ssize_t)((intptr_t)p); \
                    if ((id_as_ssize_t <= (ssize_t)INT32_MAX) && (id_as_ssize_t >= (ssize_t)INT32_MIN)) { \
                        client_data->id = (int32_t)id_as_ssize_t; \
                        touched |= CLIENT_ID_COMMAND_TOUCHED; \
                        DB_DEBUG("\tclient_data->id = %d", client_data->id) \
                        break; \
                    } \
                    DB_DEBUG("\tclient_data->id limit reached DB_CLIENT_" #action "_ID_LIMIT_REACHED(%d)", DB_CLIENT_##action##_ID_LIMIT_REACHED) \
                    err = DB_CLIENT_##action##_ID_LIMIT_REACHED; \
                    goto func##_exit1; \
                case DB_CLIENT_##action##_TECHNICIAN_ID_COMMAND: \
                    ssize_t technician_id_as_ssize_t = (ssize_t)((intptr_t)p); \
                    if ((technician_id_as_ssize_t <= (ssize_t)INT32_MAX) && (technician_id_as_ssize_t >= (ssize_t)INT32_MIN)) { \
                        client_data->technician_id = (int32_t)technician_id_as_ssize_t; \
                        touched |= CLIENT_TECHNICIAN_ID_COMMAND_TOUCHED; \
                        DB_DEBUG("\tclient_data->technician_id = %d", client_data->technician_id) \
                        break; \
                    } \
                    DB_DEBUG("\tclient_data->technician_id limit reached DB_CLIENT_" #action "_TECHNICIAN_ID_LIMIT_REACHED(%d)", DB_CLIENT_##action##_TECHNICIAN_ID_LIMIT_REACHED) \
                    err = DB_CLIENT_##action##_TECHNICIAN_ID_LIMIT_REACHED; \
                    goto func##_exit1; \
                case DB_CLIENT_##action##_CREATED_AT_COMMAND: \
                    client_data->created_at = (time_t)((intptr_t)p); \
                    touched |= CLIENT_CREATED_AT_COMMAND_TOUCHED; \
                    DB_DEBUG("\tAdded client_data->created_at = %zu", (size_t)client_data->created_at) \
                    break; \
                case DB_CLIENT_##action##_CPF_COMMAND: \
                    DB_COPY_STR_FROM(client_data->cpf, (char *)p) \
                    touched |= CLIENT_CPF_COMMAND_TOUCHED; \
                    DB_DEBUG("\tclient_data->cpf = %s", client_data->cpf) \
                    break; \
                case DB_CLIENT_##action##_NAME_COMMAND: \
                    DB_COPY_STR_FROM(client_data->name, (char *)p) \
                    touched |= CLIENT_NAME_COMMAND_TOUCHED; \
                    DB_DEBUG("\tclient_data->name = %s", client_data->name) \
                    break; \
                case DB_CLIENT_##action##_ADDRESS_COMMAND: \
                    DB_COPY_STR_FROM(client_data->address, (char *)p) \
                    touched |= CLIENT_ADDRESS_COMMAND_TOUCHED; \
                    DB_DEBUG("\tclient_data->address = %s", client_data->address) \
                    break; \
                case DB_CLIENT_##action##_DISTRICT_CITY_COMMAND: \
                    DB_COPY_STR_FROM(client_data->district_city, (char *)p) \
                    touched |= CLIENT_DISTRICT_CITY_COMMAND_TOUCHED; \
                    DB_DEBUG("\tclient_data->district_city = %s", client_data->district_city) \
                    break; \
                case DB_CLIENT_##action##_EMAIL_COMMAND: \
                    DB_COPY_STR_FROM(client_data->email, (char *)p) \
                    touched |= CLIENT_EMAIL_COMMAND_TOUCHED; \
                    DB_DEBUG("\tclient_data->email = %s", client_data->email) \
                    break; \
                case DB_CLIENT_##action##_PHONE_NUMBER_COMMAND: \
                    DB_COPY_STR_FROM(client_data->phone_number, (char *)p) \
                    touched |= CLIENT_PHONE_NUMBER_COMMAND_TOUCHED; \
                    DB_DEBUG("\tclient_data->phone_number = %s", client_data->phone_number) \
                    break; \
                default: \
                    err = DB_CLIENT_##action##_INVALID_COMMAND; \
                    DB_DEBUG("\tclient_data invalid command DB_CLIENT_" #action "_INVALID_COMMAND(%d). Command: %d", DB_CLIENT_##action##_INVALID_COMMAND, command) \
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
        DB_DEBUG(#func ": ending with status %d", err) \
        return err; \
    } \
\
    DB_DEBUG(#func ": ending with error DB_CLIENT_" #action "_INVALID") \
    return DB_CLIENT_##action##_INVALID;


#define REPAIR_MANIPULATE_HELPER(func, action, text) \
    if (repair) { \
        REPAIR_REQUEST_FLAG flag = repair->flag; \
\
        text \
\
        REPAIR_TOUCHED touched = 0; \
        va_list args; \
\
        va_start(args, repair); \
        DB_##action##_ENUM_COMMAND command; \
        void *p; \
        int err = 0; \
        while ((command = (DB_##action##_ENUM_COMMAND)va_arg(args, DB_##action##_ENUM_COMMAND))) { \
            p = (void *)va_arg(args, void *); \
            switch (command) { \
                case DB_REPAIR_##action##_CLIENT_ID: \
                    ssize_t id_as_ssize_t = (ssize_t)((intptr_t)p); \
                    if ((id_as_ssize_t <= (ssize_t)INT32_MAX) && (id_as_ssize_t >= (ssize_t)INT32_MIN)) { \
                        repair->client_id = (int32_t)id_as_ssize_t; \
                        touched |= REPAIR_CLIENT_ID_COMMAND_TOUCHED; \
                        break; \
                    } \
                    err = DB_REPAIR_##action##_CLIENT_ID_LIMIT_REACHED; \
                    goto func##_exit1; \
                case DB_REPAIR_##action##_IS_BUDGET: \
                    repair->is_bugdet = (bool)((intptr_t)p); \
                    touched |= REPAIR_IS_BUDGET_COMMAND_TOUCHED; \
                    break; \
                case DB_REPAIR_##action##_DEVICE_PROBLEM: \
                    repair->device_problem = (ELECTRONIC_DEVICE_PROBLEM)((uintptr_t)p); \
                    touched |= REPAIR_DEVICE_PROBLEM_COMMAND_TOUCHED; \
                    break; \
                case DB_REPAIR_##action##_BRAND_MODEL: \
                    DB_COPY_STR_FROM(repair->brand_model, (char *)p) \
                    touched |= REPAIR_BRAND_MODEL_COMMAND_TOUCHED; \
                    break; \
                case DB_REPAIR_##action##_SERIAL_NUMBER: \
                    DB_COPY_STR_FROM(repair->serial_number, (char *)p) \
                    touched |= REPAIR_SERIAL_MODEL_COMMAND_TOUCHED; \
                    break; \
                case DB_REPAIR_##action##_CLAIMED_DEFECT: \
                    DB_COPY_STR_FROM(repair->claimed_defect, (char *)p) \
                    touched |= REPAIR_CLAIMED_DEFECT_COMMAND_TOUCHED; \
                    break; \
                case DB_REPAIR_##action##_OBSERVATION: \
                    DB_COPY_STR_FROM(repair->observations, (char *)p) \
                    touched |= REPAIR_OBSERVATIONS_COMMAND_TOUCHED; \
                    break; \
                case DB_REPAIR_##action##_MONETARY_TYPE: \
                    repair->monetary_type = (MONETARY_TYPE)((uintptr_t)p); \
                    touched |= REPAIR_MONETARY_TYPE_COMMAND_TOUCHED; \
                    break; \
                case DB_REPAIR_##action##_EXPECTED_BUDGET_DATE: \
                    repair->expected_budget_date = (time_t)((uintptr_t)p); \
                    touched |= REPAIR_EXPECTED_BUDGET_DATE_COMMAND_TOUCHED; \
                    break; \
                case DB_REPAIR_##action##_EXPECTED_DELIVERY_DATE: \
                    repair->expected_delivery_date = (time_t)((uintptr_t)p); \
                    touched |= REPAIR_EXPECTED_DELIVERY_DATE_DATE_COMMAND_TOUCHED; \
                    break; \
                case DB_REPAIR_##action##_LABOR_BUDGET: \
                    repair->labor_budget = (int64_t)((uintptr_t)p); \
                    touched |= REPAIR_LABOR_BUDEGET_DATE_COMMAND_TOUCHED; \
                    break; \
                case DB_REPAIR_##action##_DELIVERY_DATE: \
                    repair->delivery_date = (time_t)((uintptr_t)p); \
                    touched |= REPAIR_DELIVERY_DATE_COMMAND_TOUCHED; \
                    break; \
                case DB_REPAIR_##action##_WARRANTY: \
                    repair->warranty = (time_t)((uintptr_t)p); \
                    touched |= REPAIR_WARRANTY_DATE_COMMAND_TOUCHED; \
                    break; \
                default: \
                    err = DB_REPAIR_INVALID_COMMAND; \
                    goto func##_exit1; \
            } \
        } \
\
func##_exit1: \
        va_end(args); \
\
        if (err == 0) { \
            repair->touched = touched; \
            repair->flag = flag; \
        } \
        return err; \
    } \
    return DB_REPAIR_##action##_INVALID;

#define SERVICE_MANIPULATE_HELPER(func, action, text) \
    if (service) { \
        SERVICE_REQUEST_FLAG flag = service->flag; \
\
        text \
        va_list args; \
        SERVICE_REQUEST_TOUCHED touched = 0; \
        va_start(args, service); \
        DB_##action##_ENUM_COMMAND command; \
        void *p; \
        int err = 0; \
        while ((command = (DB_##action##_ENUM_COMMAND)va_arg(args, DB_##action##_ENUM_COMMAND))) { \
            p = (void *)va_arg(args, void *); \
            switch (command) { \
                case DB_SERVICE_##action##_REPAIR_REQUEST_ID: \
                    ssize_t id_as_ssize_t = (ssize_t)((intptr_t)p); \
                    if ((id_as_ssize_t <= (ssize_t)INT32_MAX) && (id_as_ssize_t >= (ssize_t)INT32_MIN)) { \
                        service->repair_request_id = (int32_t)id_as_ssize_t; \
                        touched |= SERVICE_REPAIR_REQUEST_ID; \
                        break; \
                    } \
                    err = DB_SERVICE_##action##_REPAIR_REQUEST_ID_LIMIT_REACHED; \
                    goto func##_exit1; \
                case DB_SERVICE_##action##_QUANTITY: \
                    int32_t quantity = (int32_t)((uintptr_t)p); \
                    if (quantity > 0) { \
                        service->quantity = quantity; \
                        touched |= SERVICE_QUANTITY; \
                        break; \
                    } \
                    err = DB_SERVICE_##action##_QUANTITY_INVALID; \
                    goto func##_exit1; \
                case DB_SERVICE_##action##_MONETARY_TYPE: \
                    service->monetary_type = (MONETARY_TYPE)((uintptr_t)p); \
                    touched |= SERVICE_MONETARY_TYPE; \
                    break; \
                case DB_SERVICE_##action##_UNITY_PRICE: \
                    service->unity_price = (int64_t)((uintptr_t)p); \
                    touched |= SERVICE_UNITY_PRICE; \
                    break; \
                case DB_SERVICE_##action##_DESCRIPTION: \
                    DB_COPY_STR_FROM(service->description, (char *)p) \
                    touched |= SERVICE_DESCRIPTION; \
                    break; \
                default: \
                    err = DB_SERVICE_INVALID_COMMAND; \
                    goto func##_exit1; \
            } \
        } \
\
func##_exit1: \
        va_end(args); \
\
        if (err == 0) { \
            service->touched = touched; \
            service->flag = flag; \
        }\
        return err; \
    } \
    return DB_SERVICE_INVALID;

#endif

