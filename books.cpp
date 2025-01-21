// books.cpp
#include "books.h"
#include "database.h"
#include "utils.h"
#include <iostream>
#include <string>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>

using namespace std;
using namespace sql;

extern Connection* con;
extern PreparedStatement* pstmt;
extern ResultSet* res;

void books_menu() {
    while (true) {
        cout << "\nBooks Menu:\n";
        cout << "1. Add Book\n2. Edit Book\n3. View All Books\n4. Go Back\nChoose an option: ";
        int option;
        cin >> option;

        switch (option) {
        case 1: add_book(); break;
        case 2: edit_book(); break;
        case 3: view_books(); break;
        case 4: return;
        default: cout << "Invalid option. Try again.\n"; break;
        }
    }
}

void view_books() {
    try {
        pstmt = con->prepareStatement("SELECT * FROM books");
        res = pstmt->executeQuery();
        cout << "Books in the library:\n";
        while (res->next()) {
            cout << "ISBN: " << res->getInt("isbn") << ", Title: " << res->getString("title")
                << ", Author: " << res->getString("author") << ", Status: " << (res->getInt("available") ? "Available" : "Not Available") << endl;
        }
    }
    catch (SQLException& e) {
        cerr << "Error fetching books: " << e.what() << endl;
    }
}

void add_book() {
    string title, author;
    int isbn;

    cout << "Enter ISBN: ";
    cin >> isbn;
    cout << "Enter Title: ";
    cin.ignore();
    getline(cin, title);
    cout << "Enter Author: ";
    getline(cin, author);

    if (confirm_action("add this book")) {
        try {
            pstmt = con->prepareStatement("INSERT INTO books (isbn, title, author, available) VALUES (?, ?, ?, ?)");
            pstmt->setInt(1, isbn);
            pstmt->setString(2, title);
            pstmt->setString(3, author);
            pstmt->setInt(4, 1);
            pstmt->executeUpdate();
            cout << "Book added successfully.\n";
        }
        catch (SQLException& e) {
            cerr << "Error adding book: " << e.what() << endl;
        }
    }
}

void edit_book() {
    int isbn;
    string title, author;
    bool availability;
    bool updated = false;

    cout << "Enter ISBN of the book to edit: ";
    cin >> isbn;

    try {
        pstmt = con->prepareStatement("SELECT * FROM books WHERE isbn = ?");
        pstmt->setInt(1, isbn);
        res = pstmt->executeQuery();

        if (res->next()) {
            string current_title = res->getString("title");
            string current_author = res->getString("author");
            bool current_availability = res->getInt("available");

            cout << "Current details of the book:\n";
            cout << "Title: " << current_title << "\n";
            cout << "Author: " << current_author << "\n";
            cout << "Availability: " << (current_availability ? "Available" : "Not Available") << "\n";

            cout << "Enter new Title (or leave blank to keep the current): ";
            cin.ignore();
            getline(cin, title);

            if (!title.empty()) {
                updated = true;
            }
            else {
                title = current_title;
            }

            cout << "Enter new Author (or leave blank to keep the current): ";
            getline(cin, author);

            if (!author.empty()) {
                updated = true;
            }
            else {
                author = current_author;
            }

            cout << "Enter Availability (1 for Available, 0 for Not Available, leave blank to keep the current): ";
            string availability_input;
            getline(cin, availability_input);

            if (!availability_input.empty()) {
                availability = (availability_input == "1");
                updated = true;
            }
            else {
                availability = current_availability;
            }

            if (updated) {
                pstmt = con->prepareStatement("UPDATE books SET title = ?, author = ?, available = ? WHERE isbn = ?");
                pstmt->setString(1, title);
                pstmt->setString(2, author);
                pstmt->setInt(3, availability);
                pstmt->setInt(4, isbn);
                pstmt->executeUpdate();
                cout << "Book details updated successfully.\n";
            }
            else {
                cout << "No changes made.\n";
            }
        }
        else {
            cout << "No book found with the specified ISBN.\n";
        }
    }
    catch (SQLException& e) {
        cerr << "Error editing book: " << e.what() << endl;
    }
}

