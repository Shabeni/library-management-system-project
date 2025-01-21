// auth.cpp
#include "auth.h"
#include "database.h"
#include "student.h"
#include "books.h"
#include "users.h"
#include "roles.h"
#include "utils.h"
#include <iostream>
#include <conio.h>  
#include <iomanip>
#include <sstream>
#include <openssl/sha.h>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>

using namespace std;
using namespace sql;

extern Connection* con;
extern PreparedStatement* pstmt;
extern ResultSet* res;



// Global variable to store logged-in user's ID
int logged_in_user_id = 0;

string hashPassword(const string& password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(password.c_str()), password.size(), hash);
    stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    return ss.str();
}

bool is_username_taken(const string& username) {
    try {
        pstmt = con->prepareStatement("SELECT COUNT(*) AS user_count FROM users WHERE username = ?");
        pstmt->setString(1, username);
        res = pstmt->executeQuery();

        if (res->next() && res->getInt("user_count") > 0) {
            return true;
        }
    }
    catch (SQLException& e) {
        cerr << "Error checking username: " << e.what() << endl;
    }
    return false;
}

bool is_password_complex(const string& password) {
    if (password.length() < 8) {
        cout << "Password must be at least 8 characters long.\n";
        return false;
    }

    bool has_upper = false, has_lower = false, has_digit = false, has_special = false;

    for (char c : password) {
        if (isupper(c)) has_upper = true;
        else if (islower(c)) has_lower = true;
        else if (isdigit(c)) has_digit = true;
        else has_special = true;
    }

    if (!has_upper) cout << "Password must contain at least one uppercase letter.\n";
    if (!has_lower) cout << "Password must contain at least one lowercase letter.\n";
    if (!has_digit) cout << "Password must contain at least one digit.\n";
    if (!has_special) cout << "Password must contain at least one special character.\n";

    return has_upper && has_lower && has_digit && has_special;
}


// Function mask password input
string getPassword() {
    string password = "";
    char ch;
    while (true) {
        ch = _getch();
        if (ch == 13)
            break;
        else if (ch == 8) {
            if (password.length() > 0) {
                password.pop_back();
                cout << "\b \b";
            }
        }
        else {
            password += ch;
            cout << "*";
        }
    }
    cout << endl;
    return password;
}

void admin_login() {
    string username, password;
    int role_id, status;

    cout << "Login:\n";
    cout << "Username: ";
    cin >> username;
    cout << "Password: ";
    password = getPassword();  // Use getPassword() to mask password input

    try {
        string hashed_password = hashPassword(password);

        pstmt = con->prepareStatement("SELECT user_id, username, password, role_id, status FROM users WHERE username = ? AND password = ?");
        pstmt->setString(1, username);
        pstmt->setString(2, hashed_password);
        res = pstmt->executeQuery();

        if (res->next()) {
            status = res->getInt("status");

            if (status != 1) {
                cout << "Your account is not active. Please check with administration.\n";
                return;  // Exit the function if the account is inactive
            }

            logged_in_user_id = res->getInt("user_id");
            role_id = res->getInt("role_id");

            pstmt = con->prepareStatement("SELECT role_name FROM roles WHERE role_id = ?");
            pstmt->setInt(1, role_id);
            res = pstmt->executeQuery();

            if (res->next()) {
                string role_name = res->getString("role_name");

                if (role_name == "admin") {
                    cout << "Login successful.\n";
                    while (true) {
                        cout << "\nAdmin Menu:\n";
                        cout << "1. Books\n";
                        cout << "2. Users\n";
                        cout << "3. Roles\n";
                        cout << "4. Manage Balances\n";
                        cout << "5. Logout\n";
                        cout << "Choose an option: ";

                        int option;
                        cin >> option;

                        if (option == 5) {
                            cout << "Logging out...\n";
                            logged_in_user_id = 0;
                            clear_screen();
                            break;
                        }

                        switch (option) {
                        case 1: books_menu(); break;
                        case 2: users_menu(); break;
                        case 3: roles_menu(); break;
                        case 4: manage_balance(); break;
                        default: cout << "Invalid option.\n"; break;
                        }
                    }
                }
                else if (role_name == "student") {
                    cout << "Login successful.\n";
                    student_menu();
                }
                else {
                    cout << "Access denied. You do not have sufficient privileges.\n";
                }
            }
        }
        else {
            cout << "Incorrect username or password.\n";
        }
    }
    catch (SQLException& e) {
        cerr << "Error during login: " << e.what() << endl;
    }
}
