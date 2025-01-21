// student.cpp
#include "student.h"
#include "balance.h"
#include "database.h"
#include "utils.h"

#include <iostream>
#include <iomanip>
#include <ctime>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>

using namespace std;
using namespace sql;

extern Connection* con;
extern PreparedStatement* pstmt;
extern ResultSet* res;
extern int logged_in_user_id;
void student_menu() {
    while (true) {
        cout << "\nStudent Menu:\n";
        cout << "1. View Available Books\n";
        cout << "2. Borrow a Book\n";
        cout << "3. Return a Book\n";
        cout << "4. View Borrowed Books History\n";
        cout << "5. View Borrowed Books Not Returned\n";
        cout << "6. View Balance\n";
        cout << "7. Logout\n";
        cout << "Enter option: ";
        int option;
        cin >> option;
        if (option == 7) {
            cout << "Logging out...\n";
            logged_in_user_id = 0;
            clear_screen();
            break;
        }

        switch (option) {
        case 1: view_available_books(); break;
        case 2: borrow_book(); break;
        case 3: return_book(); break;
        case 4: view_returned_books(); break;
        case 5: view_borrowed_books_not_returned(); break;
        case 6: view_balance(); break;
        case 7: cout << "Logging out...\n"; logged_in_user_id = 0; return;
        default: cout << "Invalid option.\n"; break;
        }


    }
}

void borrow_book() {
    int isbn;
    cout << "Enter ISBN of the book to borrow: ";
    cin >> isbn;

    try {
        pstmt = con->prepareStatement("UPDATE books SET available = 0 WHERE isbn = ? AND available = 1");
        pstmt->setInt(1, isbn);
        int affected_rows = pstmt->executeUpdate();

        if (affected_rows > 0) {
            pstmt = con->prepareStatement(
                "INSERT INTO borrowed_books (user_id, isbn, borrow_date, due_date, fine_amount) "
                "VALUES (?, ?, CURDATE(), DATE_ADD(CURDATE(), INTERVAL 15 DAY), 0.00)"
            );
            pstmt->setInt(1, logged_in_user_id);
            pstmt->setInt(2, isbn);
            pstmt->executeUpdate();
            cout << "Book borrowed successfully. Due date is 15 days from today.\n";
        }
        else {
            cout << "Book is not available for borrowing.\n";
        }
    }
    catch (SQLException& e) {
        cerr << "Error borrowing book: " << e.what() << endl;
    }
}

void return_book() {
    int isbn;
    cout << "Enter ISBN of the book to return: ";
    cin >> isbn;

    try {
        pstmt = con->prepareStatement(
            "SELECT borrow_id, borrow_date, due_date "
            "FROM borrowed_books "
            "WHERE user_id = ? AND isbn = ? AND return_date IS NULL "
            "ORDER BY borrow_date DESC "
            "LIMIT 1"
        );
        pstmt->setInt(1, logged_in_user_id);
        pstmt->setInt(2, isbn);
        res = pstmt->executeQuery();

        if (res->next()) {
            string due_date_str = res->getString("due_date");
            time_t now = time(0);
            tm now_tm;
            localtime_s(&now_tm, &now);

            int year, month, day;
            int result = sscanf_s(due_date_str.c_str(), "%4d-%2d-%2d", &year, &month, &day);

            if (result != 3) {
                cerr << "Error: Failed to parse due date from database.\n";
                return;
            }

            tm due_date = {};
            due_date.tm_year = year - 1900;
            due_date.tm_mon = month - 1;
            due_date.tm_mday = day;

            time_t due_time = mktime(&due_date);
            double days_late = difftime(now, due_time) / (60 * 60 * 24);

            double fine = 0.0;
            if (days_late > 0) {
                const double DAILY_FINE_RATE = 2.0;
                fine = days_late * DAILY_FINE_RATE;
            }

            if (fine > 0) {
                cout << "This book is overdue by " << static_cast<int>(days_late) << " days. Fine: $" << fixed << setprecision(2) << fine << endl;

                pstmt = con->prepareStatement("UPDATE users SET balance = balance + ? WHERE user_id = ?");
                pstmt->setDouble(1, fine);
                pstmt->setInt(2, logged_in_user_id);
                pstmt->executeUpdate();
            }
            else {
                cout << "Book returned on time. No fine incurred.\n";
            }

            pstmt = con->prepareStatement("UPDATE books SET available = 1 WHERE isbn = ?");
            pstmt->setInt(1, isbn);
            pstmt->executeUpdate();

            pstmt = con->prepareStatement("UPDATE borrowed_books SET return_date = CURDATE(), fine_amount = ? WHERE borrow_id = ?");
            pstmt->setDouble(1, fine);
            pstmt->setInt(2, res->getInt("borrow_id"));
            pstmt->executeUpdate();

            cout << "Book returned successfully.\n";
        }
        else {
            cout << "Error: You have not borrowed this book or the ISBN is invalid.\n";
        }
    }
    catch (SQLException& e) {
        cerr << "Error returning book: " << e.what() << endl;
    }
}

void view_returned_books() {
    try {
        pstmt = con->prepareStatement(
            "SELECT books.title, borrowed_books.borrow_date, borrowed_books.due_date, "
            "borrowed_books.return_date, borrowed_books.fine_amount "
            "FROM borrowed_books "
            "JOIN books ON borrowed_books.isbn = books.isbn "
            "WHERE borrowed_books.user_id = ? AND borrowed_books.return_date IS NOT NULL");
        pstmt->setInt(1, logged_in_user_id);
        res = pstmt->executeQuery();

        cout << "\nYour Returned Books:\n";
        while (res->next()) {
            cout << "Title: " << res->getString("title") << endl;
            cout << "Borrowed Date: " << res->getString("borrow_date") << endl;
            cout << "Due Date: " << res->getString("due_date") << endl;
            cout << "Return Date: " << res->getString("return_date") << endl;
            cout << "Fine Amount: $" << fixed << setprecision(2) << res->getDouble("fine_amount") << endl;
            cout << "---------------------------\n";
        }
    }
    catch (SQLException& e) {
        cerr << "Error viewing returned books: " << e.what() << endl;
    }
}

void view_borrowed_books_not_returned() {
    try {
        pstmt = con->prepareStatement(
            "SELECT books.title, borrowed_books.borrow_date, borrowed_books.due_date, "
            "borrowed_books.return_date, borrowed_books.fine_amount "
            "FROM borrowed_books "
            "JOIN books ON borrowed_books.isbn = books.isbn "
            "WHERE borrowed_books.user_id = ? AND borrowed_books.return_date IS NULL");
        pstmt->setInt(1, logged_in_user_id);
        res = pstmt->executeQuery();

        cout << "\nYour Borrowed Books (Not Returned Yet):\n";
        while (res->next()) {
            cout << "Title: " << res->getString("title") << endl;
            cout << "Borrowed Date: " << res->getString("borrow_date") << endl;
            cout << "Due Date: " << res->getString("due_date") << endl;
            cout << "Return Date: Not returned yet" << endl;
            cout << "Fine Amount: $" << fixed << setprecision(2) << res->getDouble("fine_amount") << endl;
            cout << "---------------------------\n";
        }
    }
    catch (SQLException& e) {
        cerr << "Error viewing borrowed books (not returned yet): " << e.what() << endl;
    }
}
// View available books
void view_available_books() {
    try {
        pstmt = con->prepareStatement("SELECT * FROM books WHERE available = 1");
        res = pstmt->executeQuery();
        cout << "\nAvailable Books:\n";
        while (res->next()) {
            cout << "ISBN: " << res->getString("isbn") << ", Title: " << res->getString("title") << ", Author: " << res->getString("author") << endl;
        }
    }
    catch (SQLException& e) {
        cerr << "Error viewing available books: " << e.what() << endl;
    }
}


void view_balance() {
    if (logged_in_user_id == 0) {
        cout << "You are not logged in. Please log in first.\n";
        return;
    }

    try {
        pstmt = con->prepareStatement("SELECT balance FROM users WHERE user_id = ?");
        pstmt->setInt(1, logged_in_user_id);
        res = pstmt->executeQuery();

        if (res->next()) {
            double balance = res->getDouble("balance");
            cout << "\nYour current balance is: $" << fixed << setprecision(2) << balance << "\n";
        }
        else {
            cout << "Unable to retrieve balance. Please try again later.\n";
        }
    }
    catch (SQLException& e) {
        cerr << "Error retrieving balance: " << e.what() << endl;
    }
}