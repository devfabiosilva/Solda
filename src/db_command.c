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

void technician_delete(TECHNICIAN_DATA *technician_data)
{
    switch (technician_data->flag) {
        case TECHNICIAN_READ_FROM_DATABASE:
        case TECHNICIAN_DATA_UPDATE:
            break;
        default:
            technician_data_clear(technician_data);
            return;
    }

    technician_data->flag = TECHNICIAN_DATA_DELETE;
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

void client_delete(CLIENT_DATA *client_data)
{
    switch (client_data->flag) {
        case CLIENT_DATA_READ_FROM_DATABASE:
        case CLIENT_DATA_UPDATE:
            break;
        default:
            client_data_clear(client_data);
            return;
    }

    client_data->flag = CLIENT_DATA_DELETE;

}

int repair_add(REPAIR *repair, ...)
{
    REPAIR_MANIPULATE_HELPER(
        repair_add,
        ADD,
        switch (flag) {
            case REPAIR_REQUEST_DELETE:
            case REPAIR_REQUEST_READ_FROM_DATABASE:
                return DB_REPAIR_REQUEST_ADD_UNABLE_TO_APPEND_DATA;
            case REPAIR_REQUEST_INIT:
            case REPAIR_REQUEST_NEW_AND_DELETED_BEFORE_SAVE:
                flag = REPAIR_REQUEST_NEW;
            default:
        }
    )
}

int repair_update(REPAIR *repair, ...)
{
    REPAIR_MANIPULATE_HELPER(
        repair_update,
        UPDATE,
        switch (flag) {
            case REPAIR_REQUEST_UPDATE:
            case REPAIR_REQUEST_READ_FROM_DATABASE:
                flag = REPAIR_REQUEST_UPDATE;
                break;
            default:
                return DB_REPAIR_UNABLE_TO_UPDATE;
        }
    )
}

void repair_delete(REPAIR *repair)
{
    switch (repair->flag) {
        case REPAIR_REQUEST_READ_FROM_DATABASE:
        case REPAIR_REQUEST_UPDATE:
            break;
        default:
            repair_request_clear(repair);
            return;
    }

    repair->flag = REPAIR_REQUEST_DELETE;
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

