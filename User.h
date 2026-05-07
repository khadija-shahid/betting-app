#pragma once
#include <string>

class User {
public:
    std::string username;    
    std::string fullname;
    std::string email;
    size_t passwordHash;       

    User() {}
    User(std::string uname, std::string fname, std::string mail, size_t pHash)
        : username(uname), fullname(fname), email(mail), passwordHash(pHash) {}
};
