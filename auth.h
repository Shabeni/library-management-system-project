#pragma once
#include <string>
void admin_login();
std::string hashPassword(const std::string& password);
bool is_username_taken(const std::string& username);
bool is_password_complex(const std::string& password);

