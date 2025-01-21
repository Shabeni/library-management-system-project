#include <iostream>
#include "utils.h"

#include <iomanip>
#include <cmath>    // For rounding
#include <sstream>
#include <conio.h>  
#include <cstdlib>  // For system()
#include <string>
#include "mysql_driver.h"
#include "mysql_connection.h"
#include "cppconn/prepared_statement.h"
#include <openssl/sha.h> // For SHA256 hashing
#include <ctime> // For date calculations


using namespace std;
using namespace sql;

extern Connection* con;
extern PreparedStatement* pstmt;
extern ResultSet* res;

void manage_balance() {
    string username;
    float new_balance, adjustment;
    char choice;

    cout << "Enter Username to manage balance: ";
    cin >> username;

    try {
        // Fetch the user's current balance
        pstmt = con->prepareStatement("SELECT balance FROM users WHERE username = ?");
        pstmt->setString(1, username);
        res = pstmt->executeQuery();

        if (res->next()) {
            float current_balance = res->getDouble("balance");
            cout << "Current Balance: $" << fixed << setprecision(2) << current_balance << "\n";

            cout << "Choose an option:\n";
            cout << "1. Reset Balance\n";
            cout << "2. Add to Balance\n";
            cout << "3. Deduct from Balance\n";
            cout << "Enter your choice: ";
            cin >> choice;

            switch (choice) {
            case '1':  // Reset balance
                cout << "Enter new balance: ";
                cin >> new_balance;
                pstmt = con->prepareStatement("UPDATE users SET balance = ? WHERE username = ?");
                pstmt->setDouble(1, new_balance);
                pstmt->setString(2, username);
                pstmt->executeUpdate();
                cout << "Balance reset successfully.\n";
                break;

            case '2':  // Add to balance
                cout << "Enter amount to add: ";
                cin >> adjustment;
                pstmt = con->prepareStatement("UPDATE users SET balance = balance + ? WHERE username = ?");
                pstmt->setDouble(1, adjustment);
                pstmt->setString(2, username);
                pstmt->executeUpdate();
                cout << "Balance updated successfully.\n";
                break;

            case '3':  // Deduct from balance
                cout << "Enter amount to deduct: ";
                cin >> adjustment;
                if (current_balance >= adjustment) {
                    pstmt = con->prepareStatement("UPDATE users SET balance = balance - ? WHERE username = ?");
                    pstmt->setDouble(1, adjustment);
                    pstmt->setString(2, username);
                    pstmt->executeUpdate();
                    cout << "Balance updated successfully.\n";
                }
                else {
                    cout << "Insufficient balance.\n";
                }
                break;

            default:
                cout << "Invalid choice.\n";
            }
        }
        else {
            cout << "User not found.\n";
        }
    }
    catch (SQLException& e) {
        cerr << "Error managing balance: " << e.what() << endl;
    }
}
