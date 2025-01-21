#pragma once
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>

void init_mysql();
void cleanup_mysql();