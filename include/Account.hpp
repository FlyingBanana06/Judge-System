// Account.hpp

#ifndef ACCOUNT_HPP
#define ACCOUNT_HPP

#include <string>
#include <vector>
#include <utility>
#include <unordered_map>

class User {
    private:
        std::string username;
        std::string password;
    public:
        User() = default;
        User(std::string name, std::string passwd);

        std::string getUsername() const { return username; }
        std::string getPassword() const { return password; }
};

class AccountSystem {
        friend class JudgeSystem;
    private:
        std::unordered_map<std::string, User> userMap;
        std::string loginUser;
        std::string userDataPath;

        bool signUp();
        void userDataUpdate();
        
    protected:
        void init(const std::string& userDataPath);
        User* search(const std::string& username);
        std::pair<bool, std::string> login();
        void addUser(const std::string& username, const std::string& password);
        bool verifyPassword(User* user);

        std::string getuserLogin() const {return loginUser;}
};

#endif // ACCOUNT_HPP