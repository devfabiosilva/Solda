 #include <db.h>
 #include <stdio.h>
 #include <db_command.h>
 #include <time.h>

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
        TECHNICIAN_ADD_NAME("name ABC"),
        TECHNICIAN_ADD_EMAIL("email@email.com"),
        TECHNICIAN_ADD_PHONE_NUMBER("12345-67890"),
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

    err = CLIENT_EXECUTE_ADD(
            client_data,
            CLIENT_ADD_TECHNICIAN_ID(technician_index),
            CLIENT_ADD_CPF("111.111.111.00"),
            CLIENT_ADD_NAME("Fernanda Silva"),
            CLIENT_ADD_ADDRESS("Client Address"),
            CLIENT_ADD_DISTRICT_CITY("Rua XYZ Quadra A"),
            CLIENT_ADD_EMAIL("fernanda@email.com"),
            CLIENT_ADD_PHONE_NUMBER("(21) 91234-6789")
        )

    if (err) goto main_exit;

    printf("\nValue of %p of %s\n", client_data, client_data->name);

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

    err = REPAIR_EXECUTE_ADD(
        repair,
        REPAIR_ADD_CLIENT_ID(1234),
        REPAIR_ADD_IS_BUDGET(true),
        REPAIR_ADD_DEVICE_PROBLEM(DEVICE_DOES_NOT_TURN_ON),
        REPAIR_ADD_BRAND_MODEL("MODEL ABC"),
        REPAIR_ADD_SERIAL_NUMBER("1234567890"),
        REPAIR_ADD_CLAIMED_DEFECT("Device Problem"),
        REPAIR_ADD_OBSERVATION("Observations test"),
        REPAIR_ADD_MONETARY_TYPE(BITCOIN),
        REPAIR_ADD_EXPECTED_BUDGET_DATE(time(NULL)),
        REPAIR_ADD_EXPECTED_DELIVERY_DATE(time(NULL)),
        REPAIR_ADD_LABOR_BUDGET(18000),
        REPAIR_ADD_DELIVERY_DATE(time(NULL)),
        REPAIR_ADD_WARRANTY(time(NULL))
    )

    if (err) goto main_exit;

    printf("\nValue of %p of %s\n", repair, repair->claimed_defect);
    size_t service_request_index;
    SERVICE *service = NULL;
    if ((err = technician_acquire_service_request_from_array(
        &service_request_index, &service,
        client_data_index, technician_index, repair_request_index,
        technician_requests
    ))) goto main_exit;

    printf("\nservice index %d and pointer %p", (int)service_request_index, service);

    err = SERVICE_EXECUTE_ADD(
        service,
        SERVICE_REPAIR_REQUEST_ID(1234),
        SERVICE_QUANTITY(172),
        SERVICE_MONETARY_TYPE(US_DOLLAR),
        SERVICE_UNITY_PRICE(29030),
        SERVICE_DESCRIPTION("Service description goes here")
    )

    if (err == 0)
        printf("\nValue of %p of %s\n", service, service->description);

main_exit:
    printf("\ntechnician_requests pointer %p\n", technician_requests);
    //technician_data_requests_free(&technician_requests);
    printf("\ntechnician_requests pointer %p\n", technician_requests);
    printf("\n status %d\n", err);
    return err;
 }
