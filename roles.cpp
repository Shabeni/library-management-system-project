// Add a new role
#include "roles.h"
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
// Roles menu
void roles_menu() {
    while (true) {
        cout << "\nRoles Menu:\n";
        cout << "1. Add Role\n2. Show All Roles\n3. Toggle Role Status (Active/Inactive)\n4. Go Back\nChoose an option: ";
        int option;
        cin >> option;

        switch (option) {
        case 1: add_role(); break;
        case 2: show_all_roles(); break;
        case 3: toggle_role_status(); break;
        case 4: return; // Go back to main menu
        default: cout << "Invalid option. Try again.\n"; break;
        }
    }
}
void add_role() {
    string role_name;
    cout << "Enter Role Name: ";
    cin.ignore();  // To ignore the previous newline
    getline(cin, role_name);

    if (confirm_action("add this role")) {
        try {
            pstmt = con->prepareStatement("INSERT INTO roles (role_name, status) VALUES (?, ?)");
            pstmt->setString(1, role_name);
            pstmt->setInt(2, 1);  // Set role as active
            pstmt->executeUpdate();
            cout << "Role added successfully.\n";
        }
        catch (SQLException& e) {
            cerr << "Error adding role: " << e.what() << endl;
        }
    }
}

// Show all roles
void show_all_roles() {
    try {
        pstmt = con->prepareStatement("SELECT * FROM roles");
        res = pstmt->executeQuery();
        cout << "Roles in the system:\n";
        while (res->next()) {
            cout << "Role ID: " << res->getInt("role_id") << ", Role Name: " << res->getString("role_name")
                << ", Status: " << (res->getInt("status") ? "Active" : "Inactive") << endl;
        }
    }
    catch (SQLException& e) {
        cerr << "Error fetching roles: " << e.what() << endl;
    }
}

// Toggle role status (soft delete)
void toggle_role_status() {
    int role_id;
    cout << "Enter Role ID to toggle status: ";
    cin >> role_id;

    try {
        pstmt = con->prepareStatement("SELECT status FROM roles WHERE role_id = ?");
        pstmt->setInt(1, role_id);
        res = pstmt->executeQuery();

        if (res->next()) {
            int current_status = res->getInt("status");
            int new_status = (current_status == 1) ? 0 : 1;

            if (confirm_action("toggle the role status")) {
                pstmt = con->prepareStatement("UPDATE roles SET status = ? WHERE role_id = ?");
                pstmt->setInt(1, new_status);
                pstmt->setInt(2, role_id);
                pstmt->executeUpdate();
                cout << "Role status updated successfully.\n";
            }
        }
        else {
            cout << "Role not found.\n";
        }
    }
    catch (SQLException& e) {
        cerr << "Error toggling role status: " << e.what() << endl;
    }
}