#   Library-management-system-project
Library management system project using C++

A **C++ console application** for managing library operations with **MySQL database integration**. The system provides comprehensive functionality for both administrators and students, featuring secure authentication and financial management.

## Features

### **Authentication & Security**
- Role-based access control (Admin/Student)
- Secure password hashing (**SHA-256**)
- Input validation and SQL injection prevention
- Username uniqueness verification

### **Administrative Features**

#### Book Management
- Add, edit, and view books
- Track book availability
- Manage book borrowing records

#### User Management
- Create and modify user accounts
- Manage user roles and permissions
- View user activity history

#### Financial Management
- Automatic fine calculation for overdue books
- Balance management for student accounts
- Financial transaction tracking

### **Student Features**
- Browse available books
- Borrow and return books
- View personal borrowing history
- Check account balance and fines

## Technical Stack
- **Language**: C++
- **Database**: MySQL
- **Libraries**:
  - MySQL Connector/C++(https://dev.mysql.com/downloads/connector/cpp/)
  - OpenSSL (for password hashing)(https://slproweb.com/products/Win32OpenSSL.html)

## Usage

### Start the Application
1. Launch the application.

### Login Using Your Credentials
- **Admin Credentials** (default):
  - **Username**: `admin`
  - **Password**: `admin123A/`
  
- **Student Credentials**:
  - Student accounts can be created by the admin.
