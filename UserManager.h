#pragma once
#include <vector>
#include <string>
#include "User.h"

class UserManager {
public:
    UserManager(const std::string& filename);

    bool registerUser(const std::string& fullname,
                      const std::string& email,
                      const std::string& password);

    bool loginUser(const std::string& username,
                   const std::string& password);

    User getLoggedInUser();
    bool isLoggedIn();
    bool resetPassword(const std::string& username,
                   const std::string& email,
                   const std::string& newPassword);
std::vector<std::string> getAllUsernames();


private:
    std::string filename;
    User loggedInUser;
    bool logged = false;

    std::vector<User> loadUsers();
    void saveUser(const User& user);
    bool userExists(const std::string& fullname, const std::string& email);
    std::string generateUserID();
bool usernameExists(const std::string& username);
bool isValidGmail(const std::string& email);


};
