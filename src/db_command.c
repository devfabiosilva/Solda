#include <db_command.h>
#include <db_errors.h>
#include <stdarg.h>
#include <string.h>
#include <db_command_macros.h>
#include <db_log.h>
#include <sys/types.h>

int technician_read(TECHNICIAN_DATA *technician_data, ...)
{
    TECHNICIAN_READ_HELPER(
        technician_read,
        flag = TECHNICIAN_READ_FROM_DATABASE;
    )
}

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

extern void _technician_data_clear(TECHNICIAN_DATA *);
void technician_delete(TECHNICIAN_DATA *technician_data)
{
    if (technician_data) {
        switch (technician_data->flag) {
            case TECHNICIAN_READ_FROM_DATABASE:
            case TECHNICIAN_DATA_UPDATE:
                break;
            default:
                _technician_data_clear(technician_data);
                return;
        }

        technician_data->flag = TECHNICIAN_DATA_DELETE;
    }
}

int client_read(CLIENT_DATA *client_data, ...)
{
    CLIENT_DATA_READ_HELPER(
        client_read,
        flag = CLIENT_DATA_READ_FROM_DATABASE;
    )
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

extern void _client_data_clear(CLIENT_DATA *);
void client_delete(CLIENT_DATA *client_data)
{
    if (client_data) {
        switch (client_data->flag) {
            case CLIENT_DATA_READ_FROM_DATABASE:
            case CLIENT_DATA_UPDATE:
                break;
            default:
                _client_data_clear(client_data);
                return;
        }

        client_data->flag = CLIENT_DATA_DELETE;
    }

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

extern void _repair_request_clear(REPAIR *);
void repair_delete(REPAIR *repair)
{
    if (repair) {
        switch (repair->flag) {
            case REPAIR_REQUEST_READ_FROM_DATABASE:
            case REPAIR_REQUEST_UPDATE:
                break;
            default:
                _repair_request_clear(repair);
                return;
        }

        repair->flag = REPAIR_REQUEST_DELETE;
    }
}

int service_add(SERVICE *service, ...)
{
    SERVICE_MANIPULATE_HELPER(
        service_add,
        ADD,
        switch (flag) {
            case SERVICE_REQUEST_DELETE:
            case SERVICE_REQUEST_READ_FROM_DATABASE:
                return DB_SERVICE_REQUEST_ADD_UNABLE_TO_APPEND_DATA;
            case SERVICE_REQUEST_INIT:
            case SERVICE_REQUEST_NEW_AND_DELETED_BEFORE_SAVE:
                flag = SERVICE_REQUEST_NEW;
            default:
        }        
    )
}

int service_update(SERVICE *service, ...)
{
    SERVICE_MANIPULATE_HELPER(
        service_update,
        UPDATE,
        switch (flag) {
            case SERVICE_REQUEST_UPDATE:
            case SERVICE_REQUEST_READ_FROM_DATABASE:
                flag = SERVICE_REQUEST_UPDATE;
                break;
            default:
                return DB_SERVICE_UNABLE_TO_UPDATE;
        }        
    )
}

void service_delete(SERVICE *service)
{
    if (service) {
        switch (service->flag) {
            case SERVICE_REQUEST_READ_FROM_DATABASE:
            case SERVICE_REQUEST_UPDATE:
                break;
            default:
                explicit_bzero((void *)service, sizeof(*service));;
                return;
        }

        service->flag = SERVICE_REQUEST_DELETE;
    }
}