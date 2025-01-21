// users.cpp
#include "users.h"
#include "database.h"
#include "auth.h"
#include "utils.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>

using namespace std;
using namespace sql;

extern Connection* con;
extern PreparedStatement* pstmt;
extern ResultSet* res;

void users_menu() {
    while (true) {
        cout << "\nUsers Menu:\n";
        cout << "1. Add User\n2. Edit User\n3. Show All Users\n4. Go Back\nChoose an option: ";
        int option;
        cin >> option;

        switch (option) {
        case 1: add_user(); break;
        case 2: edit_user(); break;
        case 3: show_all_users(); break;
        case 4: return;
        default: cout << "Invalid option. Try again.\n"; break;
        }
    }
}

void add_user() {
    string name, username, password;
    int role_id;

    cout << "Enter Name: ";
    cin.ignore();
    getline(cin, name);

    while (true) {
        cout << "Enter Username: ";
        cin >> username;

        if (!is_username_taken(username)) {
            break;
        }
        cout << "This username is already taken. Please choose another.\n";
    }

    while (true) {
        cout << "Enter Password: ";
        cin >> password;

        if (is_password_complex(password)) {
            break;
        }
        cout << "Please enter a password that meets the complexity requirements.\n";
    }

    cout << "\nActive Roles:\n";
    try {
        pstmt = con->prepareStatement("SELECT role_id, role_name FROM roles WHERE status = 1");
        res = pstmt->executeQuery();

        cout << "Available roles:\n";
        while (res->next()) {
            cout << "ID: " << res->getInt("role_id") << ", Role Name: " << res->getString("role_name") << endl;
        }

        cout << "Enter Role ID: ";
        cin >> role_id;

        pstmt = con->prepareStatement("SELECT status FROM roles WHERE role_id = ?");
        pstmt->setInt(1, role_id);
        res = pstmt->executeQuery();

        if (res->next() && res->getInt("status") == 1) {
            if (confirm_action("add this user with the selected role")) {
                string hashed_password = hashPassword(password);

                if (role_id == 1) {
                    pstmt = con->prepareStatement("INSERT INTO users (name, username, password, role_id, status, balance) VALUES (?, ?, ?, ?, ?, ?)");
                    pstmt->setString(1, name);
                    pstmt->setString(2, username);
                    pstmt->setString(3, hashed_password);
                    pstmt->setInt(4, role_id);
                    pstmt->setInt(5, 1);
                    pstmt->setNull(6, sql::DataType::DOUBLE);
                }
                else {
                    float balance;
                    cout << "Enter Balance: ";
                    cin >> balance;

                    pstmt = con->prepareStatement("INSERT INTO users (name, username, password, role_id, status, balance) VALUES (?, ?, ?, ?, ?, ?)");
                    pstmt->setString(1, name);
                    pstmt->setString(2, username);
                    pstmt->setString(3, hashed_password);
                    pstmt->setInt(4, role_id);
                    pstmt->setInt(5, 1);
                    pstmt->setDouble(6, balance);
                }

                pstmt->executeUpdate();
                cout << "User added successfully.\n";
            }
        }
        else {
            cout << "Invalid role ID or the role is not active.\n";
        }
    }
    catch (SQLException& e) {
        cerr << "Error adding user: " << e.what() << endl;
    }
}

void edit_user() {
    string username, name, role_id_input, status_input, new_password;
    int role_id, status;
    bool updated = false;

    try {
        // Prompt admin to enter the username
        cout << "Enter the Username of the user to edit: ";
        cin >> username;

        // Fetch current user details using the username
        pstmt = con->prepareStatement("SELECT * FROM users WHERE username = ?");
        pstmt->setString(1, username);
        res = pstmt->executeQuery();

        if (res->next()) {
            // Extract current details
            int user_id = res->getInt("user_id");
            string current_name = res->getString("name");
            int current_role_id = res->getInt("role_id");
            int current_status = res->getInt("status");

            cout << "Current details of the user:\n";
            cout << "Name: " << current_name << "\n";
            cout << "Username: " << username << "\n";

            // Fetch current role
            pstmt = con->prepareStatement("SELECT role_name FROM roles WHERE role_id = ?");
            pstmt->setInt(1, current_role_id);
            res = pstmt->executeQuery();

            if (res->next()) {
                string current_role_name = res->getString("role_name");
                cout << "Role: " << current_role_name << "\n";
            }

            cout << "Status: " << (current_status ? "Active" : "Inactive") << "\n";

            // Ask for new name
            cout << "Enter new Name (or leave blank to keep the current): ";
            cin.ignore(); // Ignore newline from previous input
            getline(cin, name);

            // Update name if not empty
            if (!name.empty()) {
                updated = true;
            }
            else {
                name = current_name;  // Keep the old name
            }

            // Ask for new role
            cout << "Enter new Role ID (or leave blank to keep the current): ";
            getline(cin, role_id_input);

            if (!role_id_input.empty()) {
                try {
                    role_id = stoi(role_id_input);  // Convert string to integer
                    if (role_id != current_role_id) {
                        updated = true;
                    }
                }
                catch (const invalid_argument&) {
                    cout << "Invalid role ID entered.\n";
                    return;  // Stop the function if invalid role ID is entered
                }
            }
            else {
                role_id = current_role_id;  // Keep the old role ID
            }

            // Ask for new status
            cout << "Enter new Status (1 for Active, 0 for Inactive, or leave blank to keep the current): ";
            getline(cin, status_input);

            if (!status_input.empty()) {
                try {
                    status = stoi(status_input);  // Convert string to integer
                    if (status != current_status) {
                        updated = true;
                    }
                }
                catch (const invalid_argument&) {
                    cout << "Invalid status entered. Status must be 1 (Active) or 0 (Inactive).\n";
                    return;  // Stop the function if invalid status is entered
                }
            }
            else {
                status = current_status;  // Keep the old status
            }

            // Ask if the admin wants to update the password
            cout << "Do you want to update the password? (y/n): ";
            char update_password;
            cin >> update_password;

            if (tolower(update_password) == 'y') {
                cout << "Enter new Password: ";
                cin >> new_password;

                // Check password complexity
                if (!is_password_complex(new_password)) {
                    cout << "Password does not meet complexity requirements.\n";
                    return;
                }

                // Hash the password
                new_password = hashPassword(new_password);
                updated = true;
            }
            else {
                new_password = "";  // Leave password unchanged
            }

            // If any detail was updated, proceed with the update query
            if (updated) {
                pstmt = con->prepareStatement(
                    "UPDATE users SET name = ?, role_id = ?, status = ?, password = IF(? != '', ?, password) WHERE username = ?");
                pstmt->setString(1, name);
                pstmt->setInt(2, role_id);
                pstmt->setInt(3, status);
                pstmt->setString(4, new_password);  // Check if the password was updated
                pstmt->setString(5, new_password);  // Set the hashed password
                pstmt->setString(6, username);
                pstmt->executeUpdate();
                cout << "User details updated successfully.\n";
            }
            else {
                cout << "No changes made.\n";
            }
        }
        else {
            cout << "No user found with the specified username.\n";
        }
    }
    catch (SQLException& e) {
        cerr << "Error editing user: " << e.what() << endl;
    }
}


// Show all users
void show_all_users() {
    try {
        pstmt = con->prepareStatement("SELECT user_id, name, username, role_id, status, balance FROM users");
        res = pstmt->executeQuery();
        cout << "Users in the system:\n";

        while (res->next()) {
            cout << "User ID: " << res->getInt("user_id")
                << ", Name: " << res->getString("name")
                << ", Username: " << res->getString("username")
                << ", Role ID: " << res->getInt("role_id")
                << ", Status: " << (res->getInt("status") ? "Active" : "Inactive");

            // Format the balance to two decimal places
            double balance = res->getDouble("balance");
            if (!res->isNull("balance")) {
                cout << ", Balance: " << fixed << setprecision(2) << balance;
            }

            cout << endl;
        }
    }
    catch (SQLException& e) {
        cerr << "Error fetching users: " << e.what() << endl;
    }
}