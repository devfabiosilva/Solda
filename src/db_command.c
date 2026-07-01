#include <db_command.h>
#include <db_errors.h>
#include <stdarg.h>
#include <string.h>
#include <db_command_macros.h>

int technician_add(TECHNICIAN_DATA *technician_data, ...)
{
    if (technician_data) {
        TECHNICIAN_DATA_FLAG flag = technician_data->flag;
        switch (flag)
        {
            case TECHNICIAN_DATA_DELETE:
            case TECHNICIAN_READ_FROM_DATABASE:
                return DB_TECHNICIAN_ADD_UNABLE_TO_APPEND_DATA;
            case TECHNICIAN_DATA_INIT:
            case TECHNICIAN_DATA_NEW_AND_DELETED_BEFORE_SAVE:
                flag = TECHNICIAN_DATA_NEW;
            default:
        }

        va_list args;

        va_start(args, technician_data);
        void *p;
        int err = 0;
        while ((p = (void *)va_arg(args, void *))) {
            switch ((DB_ADD_ENUM_COMMAND)va_arg(args, DB_ADD_ENUM_COMMAND)) {
                case DB_TECHNICIAN_ADD_NAME_COMMAND:
                    DB_COPY_STR_FROM(technician_data->name, (char *)p);
                    break;
                case DB_TECHNICIAN_ADD_EMAIL_COMMAND:
                    DB_COPY_STR_FROM(technician_data->email, (char *)p)
                    break;
                case DB_TECHNICIAN_ADD_PHONE_NUMBER_COMMAND:
                    DB_COPY_STR_FROM(technician_data->phone_number, (char *)p)
                    break;
                case DB_TECHNICIAN_ADD_RULES:
                    technician_data->rules = (TECHNICIAN_RULES)((intptr_t)p);
                    break;
                default:
                    err = DB_TECHNICIAN_ADD_INVALID_COMMAND;
                    goto technician_add_exit1;
            }
        }
technician_add_exit1:
        va_end(args);

        if (err == 0) {
            technician_data->touched = true;
            technician_data->flag = flag;
        }

        return err;
    }

    return DB_TECHNICIAN_ADD_INVALID;
}
