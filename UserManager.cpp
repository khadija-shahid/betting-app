#include "UserManager.h"
#include "User.h"
#include "Wallet.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <functional>
#include <vector>
#include <cstdlib>   // rand
#include <ctime>     // time

// Constructor
UserManager::UserManager(const std::string& filename)
    : filename(filename), logged(false)
{
    // Seed RNG once
    static bool seeded = false;
    if (!seeded) {
        srand(static_cast<unsigned>(time(nullptr)));
        seeded = true;
    }

    // Ensure CSV has a header
    std::ifstream check(filename);
    if (!check.good()) {
        std::ofstream file(filename);
        file << "username,fullname,email,passwordHash\n";
    }
}

// --------------------
// Load all users
// --------------------
std::vector<User> UserManager::loadUsers()
{
    std::vector<User> users;
    std::ifstream file(filename);
    if (!file.is_open()) return users;

    std::string line;
    std::getline(file, line); // skip header

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string username, fullname, email, hashStr;

        std::getline(ss, username, ',');
        std::getline(ss, fullname, ',');
        std::getline(ss, email, ',');
        std::getline(ss, hashStr, ',');

        if (username.empty()) continue;

        size_t hash = std::stoull(hashStr);
        users.emplace_back(username, fullname, email, hash);
    }
    return users;
}

// --------------------
// Save user
// --------------------
void UserManager::saveUser(const User& user)
{
    std::ofstream file(filename, std::ios::app);
    if (!file.is_open()) return;

    file << user.username << ","
         << user.fullname << ","
         << user.email << ","
         << user.passwordHash << "\n";
}

// --------------------
// Check duplicate fullname+email
// --------------------
bool UserManager::userExists(const std::string& fullname,
                             const std::string& email)
{
    auto users = loadUsers();
    for (const auto& u : users) {
        if (u.fullname == fullname && u.email == email)
            return true;
    }
    return false;
}

// --------------------
// Check username uniqueness
// --------------------
bool UserManager::usernameExists(const std::string& username)
{
    auto users = loadUsers();
    for (const auto& u : users) {
        if (u.username == username)
            return true;
    }
    return false;
}

// --------------------
// Generate UNIQUE 10-digit username
// --------------------
std::string UserManager::generateUserID()
{
    std::string id;
    do {
        long long num =
            1000000000LL + (rand() % 9000000000LL); // 10 digits
        id = std::to_string(num);
    } while (usernameExists(id));

    return id;
}

// --------------------
// Register user
// --------------------
bool UserManager::registerUser(const std::string& fullname,
                               const std::string& email,
                               const std::string& password)
{
    if (!isValidGmail(email)) {
        std::cout << "Registration failed: email must end with @gmail.com\n";
        return false;
    }

    if (userExists(fullname, email)) {
        std::cout << "User already exists.\n";
        return false;
    }

    std::hash<std::string> hasher;
    size_t hashed = hasher(password);

    std::string username = generateUserID();
    User newUser(username, fullname, email, hashed);

    saveUser(newUser);

    // CREATE INITIAL WALLET HERE (ONLY ONCE)
    Wallet w;
    w.insertCurrency("BTC", 10);
    w.insertCurrency("USDT", 1000);
    w.saveToCSV("wallet.csv", username);

    loggedInUser = newUser;
    logged = true;

    std::cout << "\nRegistration successful!\n";
    std::cout << "Your username: " << username << "\n\n";

    return true;
}


// --------------------
// Login user
// --------------------
bool UserManager::loginUser(const std::string& username,
                            const std::string& password)
{
    auto users = loadUsers();
    std::hash<std::string> hasher;
    size_t hashedInput = hasher(password);

    for (const auto& u : users) {
        if (u.username == username &&
            u.passwordHash == hashedInput)
        {
            loggedInUser = u;
            logged = true;

            std::cout << "\nLogin successful!\n";
            std::cout << "Welcome, " << u.fullname << "\n\n";
            return true;
        }
    }

    std::cout << "Invalid username or password.\n";
    return false;
}

// --------------------
// Accessors
// --------------------
User UserManager::getLoggedInUser()
{
    return loggedInUser;
}

bool UserManager::isLoggedIn()
{
    return logged;
}

bool UserManager::resetPassword(const std::string& username,
                                const std::string& email,
                                const std::string& newPassword)
{
    // Enforce Gmail-only email for password reset
if (!isValidGmail(email)) {
    std::cout << "Password reset failed: email must end with @gmail.com\n";
    return false;
}

    auto users = loadUsers();
    bool found = false;

    std::hash<std::string> hasher;
    size_t newHash = hasher(newPassword);

    // Rewrite CSV safely
    std::ofstream out(filename);
    if (!out.is_open()) {
        std::cout << "ERROR: Unable to update users file.\n";
        return false;
    }

    // Write header
    out << "username,fullname,email,passwordHash\n";

    for (auto& u : users)
    {
        if (u.username == username && u.email == email)
        {
            u.passwordHash = newHash;
            found = true;
        }

        out << u.username << ","
            << u.fullname << ","
            << u.email << ","
            << u.passwordHash << "\n";
    }

    out.close();

    if (!found)
    {
        std::cout << "Password reset failed: user not found or email mismatch.\n";
        return false;
    }

    std::cout << "Password reset successful. You may now log in.\n";
    return true;
}

std::vector<std::string> UserManager::getAllUsernames()
{
    std::vector<std::string> names;
    auto users = loadUsers();

    for (const auto& u : users)
        names.push_back(u.username);

    return names;
}

bool UserManager::isValidGmail(const std::string& email)
{
    const std::string suffix = "@gmail.com";
    if (email.length() < suffix.length()) return false;

    return email.substr(email.length() - suffix.length()) == suffix;
}

