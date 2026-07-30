#include <stdio.h>
#include <db.h>
#include <stdio.h>
#include <db_command.h>
#include <time.h>
#include <tests/asserts.h>
#include <stdlib.h>
#include <db_errors.h>

int test_add_manipulation();
int test_add_invalid_manipulation();

int main(int argc, char **argv)
{
  (void)argc;
  (void)argv;

  int err = test_add_manipulation();
  if (err)
    return err;

  return test_add_invalid_manipulation();
}

static void test_add_manipulation_destroy_on_failure(void *ctx)
{
    ERROR_MSG_FMT("FAILURE: Destroying **technician_requests %p ...", (uintptr_t)ctx)
    technician_data_requests_free((TECHNICIAN_DATA_REQUESTS **)ctx);
    ERROR_MSG_FMT("Destroyed. Exiting ... %p ...", (uintptr_t)ctx)
    end_tests();
    exit(EXIT_FAILURE);
}

int test_add_manipulation()
{
    TITLE_MSG("Begin test_add_manipulation ...")
    TECHNICIAN_DATA_REQUESTS *technician_requests = NULL;
    int err = technician_data_requests_init(&technician_requests);
    if (err) {
        printf("\ntechnician_data_requests_init error %d\n", err);
        return err;
    }

    C_ASSERT_NOT_NULL(technician_requests)

    size_t technician_index;
    TECHNICIAN_DATA *technician_data = NULL;

    if ((err = technician_acquire_technician_data_from_array(
        &technician_index, &technician_data,
        technician_requests
    ))) goto test_add_manipulation_exit;

    printf("\ntechnician_data index %d and pointer %p", (int)technician_index, technician_data);

    char *expected_name_in_technician = "name ABC";
    char *expected_email_in_technician = "email@email.com";
    char *expected_phone_number_in_technician = "12345-67890";
    err = TECHNICIAN_EXECUTE_ADD(technician_data,
        TECHNICIAN_ADD_NAME(expected_name_in_technician),
        TECHNICIAN_ADD_EMAIL(expected_email_in_technician),
        TECHNICIAN_ADD_PHONE_NUMBER(expected_phone_number_in_technician),
        TECHNICIAN_ADD_RULES(IS_ROOT_ADMIN_SUPREME)
    )

    if (err) goto test_add_manipulation_exit;

    int32_t technician_touch_matched = 
        TECHNICIAN_NAME_TOUCHED | TECHNICIAN_EMAIL_TOUCHED |
        TECHNICIAN_PHONE_NUMBER_TOUCHED | TECHNICIAN_RULES_TOUCHED;
    C_ASSERT_EQUAL_U32(
        technician_touch_matched, technician_data->touched,
        CTEST_SETTER(
            CTEST_TITLE("Touched field must be match with expected with %d.", technician_touch_matched),
            CTEST_ON_ERROR_CB(test_add_manipulation_destroy_on_failure, (void *)&technician_requests)
        )
    )

    C_ASSERT_EQUAL_S32(0, technician_data->id,
        CTEST_SETTER(
            CTEST_TITLE("Check technician ID ..."),
            CTEST_ON_ERROR_CB(test_add_manipulation_destroy_on_failure, (void *)&technician_requests),
            CTEST_ON_ERROR("Failed tecnician id value. Was expected 0 but found %d", technician_data->id)
        )
    )

    C_ASSERT_EQUAL_S32(0, technician_data->version,
        CTEST_SETTER(
            CTEST_TITLE("Check technician version ..."),
            CTEST_ON_ERROR_CB(test_add_manipulation_destroy_on_failure, (void *)&technician_requests),
            CTEST_ON_ERROR("Failed tecnician version value. Was expected 0 but found %d", technician_data->version)
        )
    )

    C_ASSERT_EQUAL_INT(IS_ROOT_ADMIN_SUPREME, technician_data->rules,
        CTEST_SETTER(
            CTEST_TITLE("Check technician rules ..."),
            CTEST_ON_ERROR_CB(test_add_manipulation_destroy_on_failure, (void *)&technician_requests),
            CTEST_ON_ERROR("Failed tecnician rules value. Was expected NO_ACCESS (0) but found %d", technician_data->version)
        )
    )

    C_ASSERT_EQUAL_INT(TECHNICIAN_DATA_NEW, technician_data->flag,
        CTEST_SETTER(
            CTEST_TITLE("Check technician initialize in memory flag ..."),
            CTEST_ON_ERROR_CB(test_add_manipulation_destroy_on_failure, (void *)&technician_requests),
            CTEST_ON_ERROR("Failed tecnician flag value. Was expected TECHNICIAN_DATA_NEW (%d) but found %d", TECHNICIAN_DATA_NEW, technician_data->version)
        )
    )

    C_ASSERT_EQUAL_U64(
        0, (uint64_t)technician_data->created_at,
        CTEST_SETTER(
            CTEST_TITLE("Checking created at field in technician data ..."),
            CTEST_ON_ERROR_CB(test_add_manipulation_destroy_on_failure, (void *)&technician_requests),
            CTEST_ON_ERROR("Failed. Was expected %lu but found %lu", 0U, (uint64_t)technician_data->created_at)
        )
    )

    C_ASSERT_EQUAL_STRING(
        expected_name_in_technician, technician_data->name,
        CTEST_SETTER(
            CTEST_TITLE("Checking name field on Technician data ..."),
            CTEST_ON_ERROR_CB(test_add_manipulation_destroy_on_failure, (void *)&technician_requests),
            CTEST_ON_ERROR("Failed. Was expected %s but found %s", expected_name_in_technician, technician_data->name)
        )
    )

    C_ASSERT_EQUAL_STRING(
        expected_email_in_technician, technician_data->email,
        CTEST_SETTER(
            CTEST_TITLE("Checking email field on Technician data ..."),
            CTEST_ON_ERROR_CB(test_add_manipulation_destroy_on_failure, (void *)&technician_requests),
            CTEST_ON_ERROR("Failed. Was expected %s but found %s", expected_email_in_technician, technician_data->email)
        )
    )

    C_ASSERT_EQUAL_STRING(
        expected_phone_number_in_technician, technician_data->phone_number,
        CTEST_SETTER(
            CTEST_TITLE("Checking phone number field on Technician data ..."),
            CTEST_ON_ERROR_CB(test_add_manipulation_destroy_on_failure, (void *)&technician_requests),
            CTEST_ON_ERROR("Failed. Was expected %s but found %s", expected_phone_number_in_technician, technician_data->phone_number)
        )
    )

    C_ASSERT_NULL(technician_data->client_requests.array,
        CTEST_SETTER(
            CTEST_TITLE("Checking client_requests.array field in Technician data is NULL (lazy mode) ..."),
            CTEST_ON_ERROR_CB(test_add_manipulation_destroy_on_failure, (void *)&technician_requests),
            CTEST_ON_ERROR("Failed. Was expected NULL but NOT NULL found")
        )
    )

    C_ASSERT_EQUAL_U64(
        0, (uint64_t)technician_data->client_requests.n,
        CTEST_SETTER(
            CTEST_TITLE("Checking client_requests.n in technician data (lazy mode) ..."),
            CTEST_ON_ERROR_CB(test_add_manipulation_destroy_on_failure, (void *)&technician_requests),
            CTEST_ON_ERROR("Failed. Was expected 0 size array but found %lu", (uint64_t)technician_data->client_requests.n)
        )
    )

    C_ASSERT_EQUAL_U64(
        0, (uint64_t)technician_data->client_requests.array_max_len,
        CTEST_SETTER(
            CTEST_TITLE("Checking client_requests.array_max_len in technician data (lazy mode) ..."),
            CTEST_ON_ERROR_CB(test_add_manipulation_destroy_on_failure, (void *)&technician_requests),
            CTEST_ON_ERROR("Failed. Was expected 0 size array but found %lu", (uint64_t)technician_data->client_requests.array_max_len)
        )
    )

    size_t client_data_index;
    CLIENT_DATA *client_data = NULL;

    printf("\nValue of %p of %s\n", technician_data, technician_data->name);

    if ((err = technician_acquire_client_data_from_array(
        &client_data_index,
        &client_data,
        technician_index,
        technician_requests
    ))) goto test_add_manipulation_exit;

    printf("\nclient_data index %d and pointer %p", (int)client_data_index, client_data);

    char *client_cpf = "111.111.111.00";
    char *client_name = "Fernanda Silva";
    char *client_address = "Client Address";
    char *client_district_city = "Rua XYZ Quadra A";
    char *client_email = "fernanda@email.com";
    char *client_phone_number = "(21) 91234-6789";
    err = CLIENT_EXECUTE_ADD(
            client_data,
            CLIENT_ADD_TECHNICIAN_ID(technician_index),
            CLIENT_ADD_CPF(client_cpf),
            CLIENT_ADD_NAME(client_name),
            CLIENT_ADD_ADDRESS(client_address),
            CLIENT_ADD_DISTRICT_CITY(client_district_city),
            CLIENT_ADD_EMAIL(client_email),
            CLIENT_ADD_PHONE_NUMBER(client_phone_number)
        )

    if (err) goto test_add_manipulation_exit;

    int32_t client_touch_matched = 
        CLIENT_TECHNICIAN_ID_COMMAND_TOUCHED | CLIENT_CPF_COMMAND_TOUCHED |
        CLIENT_NAME_COMMAND_TOUCHED | CLIENT_ADDRESS_COMMAND_TOUCHED |
        CLIENT_DISTRICT_CITY_COMMAND_TOUCHED | CLIENT_EMAIL_COMMAND_TOUCHED |
        CLIENT_PHONE_NUMBER_COMMAND_TOUCHED;

    C_ASSERT_EQUAL_U32(
        client_touch_matched, client_data->touched,
        CTEST_SETTER(
            CTEST_TITLE("Checking touched in client data ..."),
            CTEST_ON_ERROR_CB(test_add_manipulation_destroy_on_failure, (void *)&technician_requests),
            CTEST_ON_ERROR("Failed. Was expected touched %d but found %d", client_touch_matched, client_data->touched),
            CTEST_ON_SUCCESS("Test touched success for client data")
        )
    )

    printf("\nValue of %p of %s\n", client_data, client_data->name);

    C_ASSERT_EQUAL_S32(0, client_data->id,
        CTEST_SETTER(
            CTEST_TITLE("Checking id field on client data ..."),
            CTEST_ON_ERROR_CB(test_add_manipulation_destroy_on_failure, (void *)&technician_requests),
            CTEST_ON_ERROR("Failed. Was expected 0 but found %d", client_data->id),
            CTEST_ON_SUCCESS("Test id field success for client data")
        )
    )

    C_ASSERT_EQUAL_S32(0, client_data->technician_id,
        CTEST_SETTER(
            CTEST_TITLE("Checking technician_id field on client data ..."),
            CTEST_ON_ERROR_CB(test_add_manipulation_destroy_on_failure, (void *)&technician_requests),
            CTEST_ON_ERROR("Failed. Was expected 0 but found %d", client_data->technician_id),
            CTEST_ON_SUCCESS("Test technician_id field success for client data")
        )
    )

    C_ASSERT_EQUAL_S32(0, client_data->version,
        CTEST_SETTER(
            CTEST_TITLE("Checking version field on client data ..."),
            CTEST_ON_ERROR_CB(test_add_manipulation_destroy_on_failure, (void *)&technician_requests),
            CTEST_ON_ERROR("Failed. Was expected 0 but found %d", client_data->version),
            CTEST_ON_SUCCESS("Test version field success for client data")
        )
    )

    C_ASSERT_EQUAL_U32(0, (uint64_t)client_data->created_at,
        CTEST_SETTER(
            CTEST_TITLE("Checking version created_at on client data ..."),
            CTEST_ON_ERROR_CB(test_add_manipulation_destroy_on_failure, (void *)&technician_requests),
            CTEST_ON_ERROR("Failed. Was expected 0 but found %lu", client_data->created_at),
            CTEST_ON_SUCCESS("Test created_at field success for client data")
        )
    )

    C_ASSERT_EQUAL_STRING(
        client_cpf, client_data->cpf,
        CTEST_SETTER(
            CTEST_TITLE("Checking cpf field on client data ..."),
            CTEST_ON_ERROR_CB(test_add_manipulation_destroy_on_failure, (void *)&technician_requests),
            CTEST_ON_ERROR("Failed. Was expected %s but found %s", client_cpf, client_data->cpf),
            CTEST_ON_SUCCESS("Test cpf field success for client data")
        )
    )

    C_ASSERT_EQUAL_STRING(
        client_name, client_data->name,
        CTEST_SETTER(
            CTEST_TITLE("Checking name field on client data ..."),
            CTEST_ON_ERROR_CB(test_add_manipulation_destroy_on_failure, (void *)&technician_requests),
            CTEST_ON_ERROR("Failed. Was expected %s but found %s", client_name, client_data->name),
            CTEST_ON_SUCCESS("Test name field success for client data")
        )
    )

    C_ASSERT_EQUAL_STRING(
        client_address, client_data->address,
        CTEST_SETTER(
            CTEST_TITLE("Checking address field on client data ..."),
            CTEST_ON_ERROR_CB(test_add_manipulation_destroy_on_failure, (void *)&technician_requests),
            CTEST_ON_ERROR("Failed. Was expected %s but found %s", client_address, client_data->address),
            CTEST_ON_SUCCESS("Test address field success for client data")
        )
    )

    C_ASSERT_EQUAL_STRING(
        client_district_city, client_data->district_city,
        CTEST_SETTER(
            CTEST_TITLE("Checking district_city field on client data ..."),
            CTEST_ON_ERROR_CB(test_add_manipulation_destroy_on_failure, (void *)&technician_requests),
            CTEST_ON_ERROR("Failed. Was expected %s but found %s", client_district_city, client_data->district_city),
            CTEST_ON_SUCCESS("Test district_city field success for client data")
        )
    )

    C_ASSERT_EQUAL_STRING(
        client_email, client_data->email,
        CTEST_SETTER(
            CTEST_TITLE("Checking email field on client data ..."),
            CTEST_ON_ERROR_CB(test_add_manipulation_destroy_on_failure, (void *)&technician_requests),
            CTEST_ON_ERROR("Failed. Was expected %s but found %s", client_email, client_data->email),
            CTEST_ON_SUCCESS("Test email field success for client data")
        )
    )

    C_ASSERT_EQUAL_STRING(
        client_phone_number, client_data->phone_number,
        CTEST_SETTER(
            CTEST_TITLE("Checking phone_number field on client data ..."),
            CTEST_ON_ERROR_CB(test_add_manipulation_destroy_on_failure, (void *)&technician_requests),
            CTEST_ON_ERROR("Failed. Was expected %s but found %s", client_phone_number, client_data->phone_number),
            CTEST_ON_SUCCESS("Test phone_number field success for client data")
        )
    )

    C_ASSERT_NULL(client_data->repair_requests.array,
        CTEST_SETTER(
            CTEST_TITLE("Checking repair_requests.array field in client data is NULL (lazy mode) ..."),
            CTEST_ON_ERROR_CB(test_add_manipulation_destroy_on_failure, (void *)&technician_requests),
            CTEST_ON_ERROR("Failed. Was expected NULL but NOT NULL found")
        )
    )

    C_ASSERT_EQUAL_U64(
        0, (uint64_t)client_data->repair_requests.n,
        CTEST_SETTER(
            CTEST_TITLE("Checking repair_requests.n in client data (lazy mode) ..."),
            CTEST_ON_ERROR_CB(test_add_manipulation_destroy_on_failure, (void *)&technician_requests),
            CTEST_ON_ERROR("Failed. Was expected 0 size array but found %lu", (uint64_t)client_data->repair_requests.n)
        )
    )

    C_ASSERT_EQUAL_U64(
        0, (uint64_t)client_data->repair_requests.array_max_len,
        CTEST_SETTER(
            CTEST_TITLE("Checking repair_requests.array_max_len in client data (lazy mode) ..."),
            CTEST_ON_ERROR_CB(test_add_manipulation_destroy_on_failure, (void *)&technician_requests),
            CTEST_ON_ERROR("Failed. Was expected 0 size array but found %lu", (uint64_t)client_data->repair_requests.array_max_len)
        )
    )

    size_t repair_request_index;
    REPAIR *repair = NULL;
    if ((err = technician_acquire_repair_request_from_array(
        &repair_request_index,
        &repair,
        client_data_index,
        technician_index,
        technician_requests
    ))) goto test_add_manipulation_exit;

    printf("\nrepair index %d and pointer %p", (int)repair_request_index, repair);

    int32_t client_id = 1234;
    bool is_budget = true;
    ELECTRONIC_DEVICE_PROBLEM device_problem = DEVICE_DOES_NOT_TURN_ON;
    char *brand_model = "MODEL ABC";
    char *serial_number = "1234567890";
    char *claimed_defect = "Device Problem";
    char *observation = "Observations test";
    time_t expected_budget_date = time(NULL) + 1;
    time_t expected_delivery_date = expected_budget_date + 1;
    int64_t labor_budget = 18000; 
    MONETARY_TYPE monetary_type = BITCOIN;
    time_t delivery_date = expected_delivery_date + 1;
    time_t warranty = delivery_date + 1;
    err = REPAIR_EXECUTE_ADD(
        repair,
        REPAIR_ADD_CLIENT_ID(client_id),
        REPAIR_ADD_IS_BUDGET(is_budget),
        REPAIR_ADD_DEVICE_PROBLEM(device_problem),
        REPAIR_ADD_BRAND_MODEL(brand_model),
        REPAIR_ADD_SERIAL_NUMBER(serial_number),
        REPAIR_ADD_CLAIMED_DEFECT(claimed_defect),
        REPAIR_ADD_OBSERVATION(observation),
        REPAIR_ADD_MONETARY_TYPE(monetary_type),
        REPAIR_ADD_EXPECTED_BUDGET_DATE(expected_budget_date),
        REPAIR_ADD_EXPECTED_DELIVERY_DATE(expected_delivery_date),
        REPAIR_ADD_LABOR_BUDGET(labor_budget),
        REPAIR_ADD_DELIVERY_DATE(delivery_date),
        REPAIR_ADD_WARRANTY(warranty)
    )

    if (err) goto test_add_manipulation_exit;

    int32_t repair_touch_matched = 
        REPAIR_CLIENT_ID_COMMAND_TOUCHED|REPAIR_IS_BUDGET_COMMAND_TOUCHED|
        REPAIR_DEVICE_PROBLEM_COMMAND_TOUCHED|REPAIR_BRAND_MODEL_COMMAND_TOUCHED|
        REPAIR_SERIAL_MODEL_COMMAND_TOUCHED|REPAIR_CLAIMED_DEFECT_COMMAND_TOUCHED|
        REPAIR_OBSERVATIONS_COMMAND_TOUCHED|REPAIR_MONETARY_TYPE_COMMAND_TOUCHED|
        REPAIR_EXPECTED_BUDGET_DATE_COMMAND_TOUCHED|REPAIR_EXPECTED_DELIVERY_DATE_DATE_COMMAND_TOUCHED|
        REPAIR_LABOR_BUDEGET_DATE_COMMAND_TOUCHED|REPAIR_DELIVERY_DATE_COMMAND_TOUCHED|
        REPAIR_WARRANTY_DATE_COMMAND_TOUCHED;

    C_ASSERT_EQUAL_U32(
        repair_touch_matched, repair->touched,
        CTEST_SETTER(
            CTEST_TITLE("Checking touched in repair ..."),
            CTEST_ON_ERROR_CB(test_add_manipulation_destroy_on_failure, (void *)&technician_requests),
            CTEST_ON_ERROR("Failed. Was expected touched %d but found %d", repair_touch_matched, repair->touched),
            CTEST_ON_SUCCESS("Test touched success for repair")
        )
    )

    C_ASSERT_EQUAL_S32(
        client_id, repair->client_id,
        CTEST_SETTER(
            CTEST_TITLE("Checking client_id in repair ..."),
            CTEST_ON_ERROR_CB(test_add_manipulation_destroy_on_failure, (void *)&technician_requests),
            CTEST_ON_ERROR("Failed. Was expected %d but found %d", client_id, repair->client_id)
        )
    )

    C_ASSERT_TRUE(
        is_budget == repair->is_bugdet,
        CTEST_SETTER(
            CTEST_TITLE("Checking is_bugdet in repair ..."),
            CTEST_ON_ERROR_CB(test_add_manipulation_destroy_on_failure, (void *)&technician_requests),
            CTEST_ON_ERROR("Failed. Was expected repair->is_bugdet = %d but found %d", is_budget, (int)repair->is_bugdet)
        )
    )

    C_ASSERT_EQUAL_INT(
        device_problem, repair->device_problem,
        CTEST_SETTER(
            CTEST_TITLE("Checking device_problem in repair ..."),
            CTEST_ON_ERROR_CB(test_add_manipulation_destroy_on_failure, (void *)&technician_requests),
            CTEST_ON_ERROR("Failed. Was expected %d but found %d", device_problem, repair->device_problem)
        )
    )

    C_ASSERT_EQUAL_STRING(
        brand_model, repair->brand_model,
        CTEST_SETTER(
            CTEST_TITLE("Checking brand_model in repair ..."),
            CTEST_ON_ERROR_CB(test_add_manipulation_destroy_on_failure, (void *)&technician_requests),
            CTEST_ON_ERROR("Failed. Was expected %s but found %s", brand_model, repair->brand_model)
        )
    )

    C_ASSERT_EQUAL_STRING(
        serial_number, repair->serial_number,
        CTEST_SETTER(
            CTEST_TITLE("Checking serial_number in repair ..."),
            CTEST_ON_ERROR_CB(test_add_manipulation_destroy_on_failure, (void *)&technician_requests),
            CTEST_ON_ERROR("Failed. Was expected %s but found %s", serial_number, repair->serial_number)
        )
    )

    C_ASSERT_EQUAL_STRING(
        claimed_defect, repair->claimed_defect,
        CTEST_SETTER(
            CTEST_TITLE("Checking claimed_defect in repair ..."),
            CTEST_ON_ERROR_CB(test_add_manipulation_destroy_on_failure, (void *)&technician_requests),
            CTEST_ON_ERROR("Failed. Was expected %s but found %s", claimed_defect, repair->claimed_defect)
        )
    )

    C_ASSERT_EQUAL_STRING(
        observation, repair->observations,
        CTEST_SETTER(
            CTEST_TITLE("Checking observation in repair ..."),
            CTEST_ON_ERROR_CB(test_add_manipulation_destroy_on_failure, (void *)&technician_requests),
            CTEST_ON_ERROR("Failed. Was expected %s but found %s", observation, repair->observations)
        )
    )

    C_ASSERT_EQUAL_U64(
        expected_budget_date, repair->expected_budget_date,
        CTEST_SETTER(
            CTEST_TITLE("Checking budget_date in repair ..."),
            CTEST_ON_ERROR_CB(test_add_manipulation_destroy_on_failure, (void *)&technician_requests),
            CTEST_ON_ERROR("Failed. Was expected %lu but found %lu", expected_budget_date, repair->expected_budget_date)
        )
    )

    C_ASSERT_EQUAL_U64(
        expected_delivery_date, repair->expected_delivery_date,
        CTEST_SETTER(
            CTEST_TITLE("Checking expected_delivery_date in repair ..."),
            CTEST_ON_ERROR_CB(test_add_manipulation_destroy_on_failure, (void *)&technician_requests),
            CTEST_ON_ERROR("Failed. Was expected %lu but found %lu", expected_delivery_date, repair->expected_delivery_date)
        )
    )

    C_ASSERT_EQUAL_S64(
        labor_budget, repair->labor_budget,
        CTEST_SETTER(
            CTEST_TITLE("Checking labor_bugdet in repair ..."),
            CTEST_ON_ERROR_CB(test_add_manipulation_destroy_on_failure, (void *)&technician_requests),
            CTEST_ON_ERROR("Failed. Was expected %lu but found %lu", labor_budget, repair->labor_budget)
        )
    )

    C_ASSERT_EQUAL_INT(
        monetary_type, repair->monetary_type,
        CTEST_SETTER(
            CTEST_TITLE("Checking monetary_type in repair ..."),
            CTEST_ON_ERROR_CB(test_add_manipulation_destroy_on_failure, (void *)&technician_requests),
            CTEST_ON_ERROR("Failed. Was expected %d but found %d", monetary_type, repair->monetary_type)
        )
    )

    C_ASSERT_EQUAL_U64(
        delivery_date, repair->delivery_date,
        CTEST_SETTER(
            CTEST_TITLE("Checking delivery_date in repair ..."),
            CTEST_ON_ERROR_CB(test_add_manipulation_destroy_on_failure, (void *)&technician_requests),
            CTEST_ON_ERROR("Failed. Was expected %lu but found %lu", delivery_date, repair->delivery_date)
        )
    )

    C_ASSERT_EQUAL_U64(
        warranty, repair->warranty,
        CTEST_SETTER(
            CTEST_TITLE("Checking warranty in repair ..."),
            CTEST_ON_ERROR_CB(test_add_manipulation_destroy_on_failure, (void *)&technician_requests),
            CTEST_ON_ERROR("Failed. Was expected %lu but found %lu", warranty, repair->warranty)
        )
    )

    size_t service_request_index;
    SERVICE *service = NULL;
    if ((err = technician_acquire_service_request_from_array(
        &service_request_index, &service,
        client_data_index, technician_index, repair_request_index,
        technician_requests
    ))) goto test_add_manipulation_exit;

    printf("\nservice index %d and pointer %p", (int)service_request_index, service);

    int request_id = 567890;
    int32_t quantity = 172;
    MONETARY_TYPE servic_monetary_type = US_DOLLAR;
    int64_t service_unity_price = 29030;
    char *service_description = "Service description goes here";
    err = SERVICE_EXECUTE_ADD(
        service,
        SERVICE_REPAIR_REQUEST_ID(request_id),
        SERVICE_QUANTITY(quantity),
        SERVICE_MONETARY_TYPE(servic_monetary_type),
        SERVICE_UNITY_PRICE(service_unity_price),
        SERVICE_DESCRIPTION(service_description)
    )

    if (err != 0)
        goto test_add_manipulation_exit;

    printf("\nValue of %p of %s\n", service, service->description);

    int32_t service_touch_matched = 
        SERVICE_REPAIR_REQUEST_ID|SERVICE_QUANTITY|
        SERVICE_MONETARY_TYPE|SERVICE_UNITY_PRICE|SERVICE_DESCRIPTION;

    C_ASSERT_EQUAL_U32(
        service_touch_matched, service->touched,
        CTEST_SETTER(
            CTEST_TITLE("Checking touched in service ..."),
            CTEST_ON_ERROR_CB(test_add_manipulation_destroy_on_failure, (void *)&technician_requests),
            CTEST_ON_ERROR("Failed. Was expected touched %d but found %d", service_touch_matched, service->touched),
            CTEST_ON_SUCCESS("Test touched success for service")
        )
    )

test_add_manipulation_exit:
    printf("\ntechnician_requests pointer before free %p\n", technician_requests);
    technician_data_requests_free(&technician_requests);
    printf("\ntechnician_requests pointer after free %p\n", technician_requests);
    printf("\n status %d\n", err);
    C_ASSERT_EQUAL_INT(0, err)
    TITLE_MSG("End test_add_manipulation ...")
    end_tests();
    return err;
}

int test_add_invalid_manipulation()
{
    TITLE_MSG("Begin test_add_invalid_manipulation ...")
    TECHNICIAN_DATA_REQUESTS *technician_requests = NULL;
    int err = technician_data_requests_init(&technician_requests);
    if (err) {
        printf("\ntest_add_invalid_manipulation: technician_data_requests_init error %d\n", err);
        return err;
    }

    C_ASSERT_NOT_NULL(technician_requests)

    size_t technician_index;
    TECHNICIAN_DATA *technician_data = NULL;

    if ((err = technician_acquire_technician_data_from_array(
        &technician_index, &technician_data,
        technician_requests
    ))) goto test_add_invalid_manipulation_exit;

    printf("\ntest_add_invalid_manipulation: technician_data index %d and pointer %p", (int)technician_index, technician_data);

    err = technician_add(technician_data, 123456, "Wrong parameter", 0, NULL);

    if (err == DB_TECHNICIAN_ADD_INVALID_COMMAND)
      err = 0;
    else {
        C_ASSERT_FAIL(
            CTEST_SETTER(
                CTEST_TITLE("technician_add failed to check invalid command ..."),
                CTEST_ON_ERROR_CB(test_add_manipulation_destroy_on_failure, (void *)&technician_requests),
                CTEST_ON_ERROR("Failed. Was expected DB_TECHNICIAN_ADD_INVALID_COMMAND(%d) but found %d", DB_TECHNICIAN_ADD_INVALID_COMMAND, err)
            )
        )
    }

test_add_invalid_manipulation_exit:
    printf("\ntest_add_invalid_manipulation: technician_requests pointer before free %p\n", technician_requests);
    technician_data_requests_free(&technician_requests);
    printf("\ntest_add_invalid_manipulation: technician_requests pointer after free %p\n", technician_requests);
    TITLE_MSG("\ntest_add_invalid_manipulation: End test_add_manipulation ...")
    end_tests();
    return err;
}
