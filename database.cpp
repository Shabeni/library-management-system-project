#include "database.h"
#include <mysql_driver.h>
#include <mysql_connection.h>

using namespace sql;

Connection* con = nullptr;
PreparedStatement* pstmt = nullptr;
ResultSet* res = nullptr;

void init_mysql() {
    sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
    con = driver->connect("tcp://127.0.0.1:3306", "root", "");
    con->setSchema("library");
}

void cleanup_mysql() {
    if (res != nullptr) delete res;
    if (pstmt != nullptr) delete pstmt;
    if (con != nullptr) delete con;
}