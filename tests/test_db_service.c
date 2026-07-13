#include <db_service.h>

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

  //TODO implement data access in PostgresSQL
  db_service_free(&db_service);

  return err;
}