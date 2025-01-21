#include "utils.h"
#include <iostream>
#include <cstdlib>

using namespace std;

void clear_screen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

bool confirm_action(const string& action) {
    char confirmation;
    cout << "Are you sure you want to " << action << "? (y/n): ";
    cin >> confirmation;
    return (confirmation == 'y' || confirmation == 'Y');
}