#include <db_log.h>

void test_db_log();

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    test_db_log();
    return 0;
}

void test_db_log()
{
    _db_log(LOG_INFO, "Information test %d", 1256);
    _db_log(LOG_ERROR, "ERROR LOG test %d", 7890);
    _db_log(LOG_WARN, "WARN LOG test %d", 3517);
    _db_log(LOG_DEBUG, "DEBUG TEST");

    DB_INFO("Testing info with string \"%s\"", "text")
    DB_WARN("Testing WARN")
    DB_ERROR("Testing ERROR number %d", 1)
    DB_DEBUG("Testing DEBUG")

#ifndef SOLDA_DEBUG
  #warning "Solda debug is not enabled. DB_DEBUG will not work"
#endif
}
