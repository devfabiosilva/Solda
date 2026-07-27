#include <db_service.h>
#include <db_log.h>
#include <stdlib.h>

int test_connection();

int main(int argc, char **argv)
{
  (void)argc;
  (void)argv;

  return test_connection();
}

int test_connection()
{
  DB_SERVICE *db_service = NULL;

  int err = db_service_init(
    &db_service,
    "host=localhost "
    "dbname=solda_mvp "
    "user=fabio "
    "password=1234"
  );

  if (err)
    return err;

  err = db_service_load_technicians(db_service, 25, 1);

  if (err) {
    DB_ERROR("db_service_load_technicians error: %d - %s", err, DB_MESSAGE(db_service))
  }

  char *json = NULL;
  size_t json_len;

  err = db_service_load_technicians_json(&json, &json_len, db_service, 25, 1);

  if (err) {
    DB_ERROR("db_service_load_technicians_json error: %d - %s", err, DB_MESSAGE(db_service))
  } else {
    printf("\nJSON success %zu size %s\n", json_len, json);
    free(json);
  }

  json = NULL;
  err = db_service_load_clients_json(&json, &json_len, db_service, 25, 1);

  if (err) {
    DB_ERROR("db_service_load_technicians_json error: %d - %s", err, DB_MESSAGE(db_service))
  } else {
    printf("\nJSON success for clients with %zu size %s\n", json_len, json);
    free(json);
  }

  err = db_service_load_clients(db_service, 25, 1);

  if (err) {
    DB_ERROR("db_service_load_clients error: %d - %s", err, DB_MESSAGE(db_service))
  }

  json = NULL;
  err = db_service_load_repair_requests_json(&json, &json_len, db_service, 50, 1);
  if (err) {
    DB_ERROR("db_service_load_repair_requests_json error: %d - %s", err, DB_MESSAGE(db_service))
  } else {
    printf("\nJSON repair requests success for clients with %zu size %s\n", json_len, json);
    free(json);
  }

  db_service_free(&db_service);

  return err;
}
