#include <db_command.h>
#include <db_errors.h>
#include <stdarg.h>
#include <string.h>
#include <db_command_macros.h>

int technician_add(TECHNICIAN_DATA *technician_data, ...)
{
    TECHNICIAN_MANIPULATE_HELPER(
        technician_add,
        ADD,
        switch (flag) {
            case TECHNICIAN_DATA_DELETE:
            case TECHNICIAN_READ_FROM_DATABASE:
                return DB_TECHNICIAN_ADD_UNABLE_TO_APPEND_DATA;
            case TECHNICIAN_DATA_INIT:
                flag = TECHNICIAN_DATA_NEW;
            default:
        }
    )
}

int technician_update(TECHNICIAN_DATA *technician_data, ...)
{
    TECHNICIAN_MANIPULATE_HELPER(
        technician_update,
        UPDATE,
        switch (flag) {
            case TECHNICIAN_READ_FROM_DATABASE:
            case TECHNICIAN_DATA_UPDATE:
                flag = TECHNICIAN_DATA_UPDATE;
                break;
            default:
                return DB_TECHNICIAN_DATA_UNABLE_TO_UPDATE;
        }
    )
}

int technician_delete(TECHNICIAN_DATA *technician_data)
{
    switch (technician_data->flag) {
        case TECHNICIAN_READ_FROM_DATABASE:
        case TECHNICIAN_DATA_UPDATE:
            break;
        default:
            technician_data_clear(technician_data);
            return 0;
    }

    technician_data->flag = TECHNICIAN_DATA_DELETE;

    return 0;
}

int client_add(CLIENT_DATA *client_data, ...)
{
    CLIENT_DATA_MANIPULATE_HELPER(
        client_add,
        ADD, 
        switch (flag) {
            case CLIENT_DATA_DELETE:
            case CLIENT_DATA_READ_FROM_DATABASE:
                return DB_CLIENT_ADD_UNABLE_TO_APPEND_DATA;
            case CLIENT_DATA_INIT:
            case CLIENT_DATA_NEW_AND_DELETED_BEFORE_SAVE:
                flag = CLIENT_DATA_NEW;
            default:
        }
    )
}

int client_update(CLIENT_DATA *client_data, ...)
{
    CLIENT_DATA_MANIPULATE_HELPER(
        client_update,
        UPDATE, 
        switch (flag) {
            case CLIENT_DATA_READ_FROM_DATABASE:
            case CLIENT_DATA_UPDATE:
                flag = CLIENT_DATA_UPDATE;
                break;
            default:
                return DB_CLIENT_DATA_UNABLE_TO_UPDATE;
        }
    )
}

int client_delete(CLIENT_DATA *client_data)
{
    switch (client_data->flag) {
        case CLIENT_DATA_READ_FROM_DATABASE:
        case CLIENT_DATA_UPDATE:
            break;
        default:
            client_data_clear(client_data);
            return 0;
    }

    client_data->flag = CLIENT_DATA_DELETE;

    return 0;
}

int repair_add(REPAIR *repair, ...)
{
    if (repair) {
        REPAIR_REQUEST_FLAG flag = repair->flag;

        switch (flag) {
            case REPAIR_REQUEST_DELETE:
            case REPAIR_REQUEST_READ_FROM_DATABASE:
                return DB_REPAIR_REQUEST_ADD_UNABLE_TO_APPEND_DATA;
            case REPAIR_REQUEST_INIT:
            case REPAIR_REQUEST_NEW_AND_DELETED_BEFORE_SAVE:
                flag = REPAIR_REQUEST_NEW;
            default:
        }

        REPAIR_TOUCHED touched = 0;
        va_list args;

        va_start(args, repair);
        DB_ADD_ENUM_COMMAND command;
        void *p;
        int err = 0;
        while ((command = (DB_ADD_ENUM_COMMAND)va_arg(args, DB_ADD_ENUM_COMMAND))) {
            p = (void *)va_arg(args, void *);
            switch (command) {
                case DB_REPAIR_ADD_CLIENT_ID:
                    size_t id_as_size_t = (size_t)((intptr_t)p);
                    if (id_as_size_t <= INT32_MAX) {
                        repair->client_id = (int32_t)id_as_size_t;
                        touched |= REPAIR_CLIENT_ID_COMMAND_TOUCHED;
                        break;
                    }
                    err = DB_REPAIR_ADD_CLIENT_ID_LIMIT_REACHED;
                    goto repair_add_exit1;
                case DB_REPAIR_ADD_IS_BUDGET:
                    repair->is_bugdet = (bool)((intptr_t)p);
                    touched |= REPAIR_IS_BUDGET_COMMAND_TOUCHED;
                    break;
                case DB_REPAIR_ADD_DEVICE_PROBLEM:
                    repair->device_problem = (ELECTRONIC_DEVICE_PROBLEM)((uintptr_t)p);
                    touched |= REPAIR_DEVICE_PROBLEM_COMMAND_TOUCHED; 
                    break;
                case DB_REPAIR_ADD_BRAND_MODEL:
                    DB_COPY_STR_FROM(repair->brand_model, (char *)p)
                    touched |= REPAIR_BRAND_MODEL_COMMAND_TOUCHED;
                    break;
                case DB_REPAIR_ADD_SERIAL_NUMBER:
                    DB_COPY_STR_FROM(repair->serial_number, (char *)p)
                    touched |= REPAIR_SERIAL_MODEL_COMMAND_TOUCHED;
                    break;
                case DB_REPAIR_ADD_CLAIMED_DEFECT:
                    DB_COPY_STR_FROM(repair->claimed_defect, (char *)p)
                    touched |= REPAIR_CLAIMED_DEFECT_COMMAND_TOUCHED;
                    break;
                case DB_REPAIR_ADD_OBSERVATION:
                    DB_COPY_STR_FROM(repair->observations, (char *)p)
                    touched |= REPAIR_OBSERVATIONS_COMMAND_TOUCHED;
                    break;
                case DB_REPAIR_ADD_MONETARY_TYPE:
                    repair->monetary_type = (MONETARY_TYPE)((uintptr_t)p);
                    touched |= REPAIR_MONETARY_TYPE_COMMAND_TOUCHED;
                    break;
                case DB_REPAIR_ADD_EXPECTED_BUDGET_DATE:
                    repair->expected_budget_date = (time_t)((uintptr_t)p);
                    touched |= REPAIR_EXPECTED_BUDGET_DATE_COMMAND_TOUCHED;
                    break;
                case DB_REPAIR_ADD_EXPECTED_DELIVERY_DATE:
                    repair->expected_delivery_date = (time_t)((uintptr_t)p);
                    touched |= REPAIR_EXPECTED_DELIVERY_DATE_DATE_COMMAND_TOUCHED;
                    break;
                case DB_REPAIR_ADD_LABOR_BUDGET:
                    repair->labor_bugdet = (int64_t)((uintptr_t)p);
                    touched |= REPAIR_LABOR_BUDEGET_DATE_COMMAND_TOUCHED;
                    break;
                case DB_REPAIR_ADD_DELIVERY_DATE:
                    repair->delivery_date = (time_t)((uintptr_t)p);
                    touched |= REPAIR_DELIVERY_DATE_COMMAND_TOUCHED;
                    break;
                case DB_REPAIR_ADD_WARRANTY:
                    repair->warranty = (time_t)((uintptr_t)p);
                    touched |= REPAIR_WARRANTY_DATE_COMMAND_TOUCHED;
                    break;
                default:
                    err = DB_REPAIR_INVALID_COMMAND;
                    goto repair_add_exit1;
            }
        }

repair_add_exit1:
        va_end(args);

        if (err == 0) {
            repair->touched = touched;
            repair->flag = flag;
        }
        return 0;
    }
    return DB_REPAIR_ADD_INVALID;
}

int service_add(SERVICE *service, ...)
{
    if (service) {
        SERVICE_REQUEST_FLAG flag = service->flag;

        switch (flag) {
            case SERVICE_REQUEST_DELETE:
            case SERVICE_REQUEST_READ_FROM_DATABASE:
                return DB_SERVICE_REQUEST_ADD_UNABLE_TO_APPEND_DATA;
            case SERVICE_REQUEST_INIT:
            case SERVICE_REQUEST_NEW_AND_DELETED_BEFORE_SAVE:
                flag = SERVICE_REQUEST_NEW;
            default:
        }
        va_list args;

        va_start(args, service);
        DB_ADD_ENUM_COMMAND command;
        void *p;
        int err = 0;
        while ((command = (DB_ADD_ENUM_COMMAND)va_arg(args, DB_ADD_ENUM_COMMAND))) {
            p = (void *)va_arg(args, void *);
            switch (command) {
                case DB_SERVICE_ADD_REPAIR_REQUEST_ID:
                    size_t id_as_size_t = (size_t)((intptr_t)p);
                    if (id_as_size_t <= INT32_MAX) {
                        service->repair_request_id = (int32_t)id_as_size_t;
                        break;
                    }
                    err = DB_SERVICE_ADD_REPAIR_REQUEST_ID_LIMIT_REACHED;
                    goto service_add_exit1;
                case DB_SERVICE_ADD_QUANTITY:
                    int32_t quantity = (int32_t)((uintptr_t)p);
                    if (quantity > 0) {
                        service->quantity = quantity;
                        break;
                    }
                    err = DB_SERVICE_ADD_QUANTITY_INVALID;
                    goto service_add_exit1;
                case DB_SERVICE_ADD_MONETARY_TYPE:
                    service->monetary_type = (MONETARY_TYPE)((uintptr_t)p);
                    break;
                case DB_SERVICE_ADD_UNITY_PRICE:
                    service->unity_price = (int64_t)((uintptr_t)p);
                    break;
                case DB_SERVICE_ADD_DESCRIPTION:
                    DB_COPY_STR_FROM(service->description, (char *)p)
                    break;
                default:
                    err = DB_SERVICE_INVALID_COMMAND;
                    goto service_add_exit1;
            }
        }

service_add_exit1:
        va_end(args);

        if (err == 0) {
            service->touched = true;
            service->flag = flag;
        }
        return 0;
    }
    return DB_SERVICE_INVALID;
}

