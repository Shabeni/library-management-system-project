#include <iostream>
#include "database.h"
#include "auth.h"

using namespace std;

int main() {
    try {
        init_mysql();

        while (true) {
            admin_login();
        }

        cleanup_mysql();
    }
    catch (exception& e) {
        cerr << "Error: " << e.what() << endl;
    }

    return 0;
}