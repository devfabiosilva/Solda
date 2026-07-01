 #include <db.h>
 #include <stdio.h>
 #include <db_command.h>

 int main(int argc, char **argv)
 {
    TECHNICIAN_DATA_REQUESTS *technician_requests = NULL;
    int err = technician_data_requests_init(&technician_requests);
    if (err) {
        printf("\ntechnician_data_requests_init error %d\n", err);
        return err;
    }

    size_t technician_index;
    TECHNICIAN_DATA *technician_data = NULL;

    if ((err = technician_acquire_technician_data_from_array(
        &technician_index, &technician_data,
        technician_requests
    ))) goto main_exit;

    printf("\ntechnician_data index %d and pointer %p", (int)technician_index, technician_data);

    err = TECHNICIAN_EXECUTE_ADD(technician_data,
        TECHNICIAN_NAME("name ABC"),
        TECHNICIAN_EMAIL("email@email.com"),
        TECHNICIAN_PHONE_NUMBER("12345-67890"),
        TECHNICIAN_ADD_RULES(IS_ROOT_ADMIN_SUPREME)
    )

    if (err) goto main_exit;

    size_t client_data_index;
    CLIENT_DATA *client_data = NULL;

    printf("\nValue of %p of %s\n", technician_data, technician_data->name);

    if ((err = technician_acquire_client_data_from_array(
        &client_data_index,
        &client_data,
        technician_index,
        technician_requests
    ))) goto main_exit;

    printf("\nclient_data index %d and pointer %p", (int)client_data_index, client_data);

    size_t repair_request_index;
    REPAIR *repair = NULL;
    if ((err = technician_acquire_repair_request_from_array(
        &repair_request_index,
        &repair,
        client_data_index,
        technician_index,
        technician_requests
    ))) goto main_exit;

    printf("\nrepair index %d and pointer %p", (int)repair_request_index, repair);

    size_t service_request_index;
    SERVICE *service = NULL;
    if ((err = technician_acquire_service_request_from_array(
        &service_request_index, &service,
        client_data_index, technician_index, repair_request_index,
        technician_requests
    ))) goto main_exit;

    printf("\nservice index %d and pointer %p", (int)service_request_index, service);

main_exit:
    printf("\ntechnician_requests pointer %p\n", technician_requests);
    technician_data_requests_free(&technician_requests);
    printf("\ntechnician_requests pointer %p\n", technician_requests);
    printf("\n status %d\n", err);
    return err;
 }
