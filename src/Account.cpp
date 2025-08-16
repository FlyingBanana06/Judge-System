// AccountSystem.cpp

#include <iostream>
#include <fstream>
#include <utility>
#include <sstream>
#include "Account.hpp"
#include "ColorPrint.hpp"
#include "Utils.hpp"

User::User(std::string name, std::string pwd)
    : username(std::move(name)), password(std::move(pwd)) {}

// 透過 std::ifstream 來讀入資料，並將每一行的字串以 , 分隔出使用者名稱與密碼
void AccountSystem::init(const std::string& userDataPath) {
    AccountSystem::userDataPath = userDataPath;

     // 嘗試開啟 userDataPath 檔案，並讀取使用者資料。
    try {
        std::ifstream file(userDataPath); // 開啟檔案
        if( !file ) { // 若檔案不存在，拋出例外。
            throw std::runtime_error("Error: File does not exist - " + userDataPath);
        }

        // 從 userDataPath 載入使用者資料並插入數據
        std::string readLine;
        userMap.clear(); // 載入前重置 userMap
        while (getline(file, readLine)) {
            std::stringstream ss(readLine); // 將字串放進 sstream
            std::string username, password;
            // 以 ',' 分隔字串，取得 username 與 password
            getline(ss, username, ',');
            getline(ss, password);
            // 將 username 與 password 放入 userMap
            userMap[username] = User(username, password);
        }

        file.close();
    }
    catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << "\n";
    }
}

// 登入與註冊功能，引導使用者輸入相關資訊完成登入動作。
// 回傳的型態為 pair<bool,string>，first 表示的為是否登入成功，而 second 則是該使用者名稱。
std::pair<bool, std::string> AccountSystem::login() {
    while (true) {
        // 引導使用者輸入名稱。
        std::cout << blue("User Name (Enter -1 to sign up): ");
        std::string username;
        std::cin >> username;

        // 如果使用者輸入 -1 則呼叫 AccountSystem::signUp 並回傳 (false, "")。
        if (username == "-1") {
            while(signUp());
            return std::make_pair(false, "");
        }

        // 使用者輸入名稱後，查詢使用者是否存在，如果不存在必須提示使用者重新輸入。
        User* targetUser = search(username);
        if (targetUser == nullptr) {
            std::cout << red("User does not exist!\n");
            continue; // 重新輸入
        }

        // 使用者名稱存在時，要求使用者輸入密碼。        
        std::cout << green("Welcome back, ") << username << ".\n";
        if (verifyPassword(targetUser)) {
            ClearScreen();
            // 密碼輸入正確後提示登入成功，並回傳 (true, username)，將 AccountSystem::loginUser 更新。
            std::cout << yellow("Login Success!!!\n");
            loginUser = username;
            return std::make_pair(true, username);
        }
        // 如果密碼輸入失敗，則回到輸入使用者名稱。
    }
}

// 呼叫時給予使用者名稱作為參數，如果 userList 存在該使用者則回傳其指標，找不到則回傳 nullptr。
User* AccountSystem::search(const std::string& username) {
    auto it = userMap.find(username);
    return (it != userMap.end()) ? &(it->second) : nullptr;
}

// 驗證使用者密碼，若密碼正確則回傳 true，否則提示使用者重新輸入。
bool AccountSystem::verifyPassword(User* user) {
    int attempts = 0;
    std::string pwd;
    while (attempts < 3) {
        std::cout << cyan("Please enter your password: ");
        std::cin >> pwd;
        if (pwd == user->getPassword()) return true; // 密碼正確，回傳 true。
        std::cout << red("Password incorrect... please try again.\n"); //  // 密碼錯誤：提示使用者重新輸入。
        attempts++;
    }

    // 如果連續錯三次，則回傳 false。
    std::cout << yellow("Too many failed login attempts...\n\n");
    return false;
}

// 註冊功能，提示使用者依序輸入對應資訊 (名稱與密碼)。
bool AccountSystem::signUp() {
    std::string username, pwd1, pwd2;
    std::cout << green("Welcome! please enter your name: ");
    std::cin >> username;

    // 如果使用者名稱為空，則提示使用者重新輸入。
    if (username.empty()) {
        std::cout << red("Username cannot be empty.\n");
        return true;
    } 
    // 如果使用者名稱為 admin 或 -1，則提示使用者重新輸入。
    else if (username == "admin" || username == "-1") {
        std::cout << red("Username is reserved. Please choose another name.\n");
        return true;
    }
    // 如果使用者名稱已存在，則提示使用者重新輸入。
    else if (search(username) != nullptr) {
        std::cout << red("Username already exists. Please try another one.\n");
        return true;
    }


    // 輸入密碼時必須輸入兩次並驗證兩次輸入的密碼是否一樣，如果不一樣必須提示使用者重新輸入密碼。
    while (true) {
        std::cout << cyan("Please enter your password: ");
        std::cin >> pwd1;
        std::cout << cyan("Please enter your password again: ");
        std::cin >> pwd2;

        // 如果兩次輸入的密碼相同則跳出迴圈，否則提示使用者重新輸入。
        if (pwd1 == pwd2) break;
        std::cout << red("The two passwords do not match. Please try again.\n");
    }

    std::cout << yellow("Sign-up success! Please login now.\n\n");
    addUser(username, pwd2); // 更新使用者資訊。
    return false; // 註冊成功。
}

// 將解析出來的 username 與 password 放入 AccountSystem 的 member userList。
void AccountSystem::addUser(const std::string& username, const std::string& password) {
    userMap[username] = User(username, password);
    userDataUpdate(); // 呼叫 AccountSystem
}

// 將所有存在 userList 的資料重新覆蓋 (寫入) userDataPath。
void AccountSystem::userDataUpdate() {
    try {
        std::ofstream outputFile(AccountSystem::userDataPath);
        if( !outputFile ) {
            throw std::runtime_error("Error: File does not exist - " + userDataPath);
        }

        // 遍歷 userList ，透過 outputFile 寫入 userDataPath。
        // 需要 class User 的 getter 才能取得 Username 和 Password。
        for (const auto& pair : userMap) { // 遍歷 userList 裡的每個 User
            outputFile << pair.second.getUsername() << "," << pair.second.getPassword() << "\n"; // 將 user 寫入檔案
        }
        outputFile.close();
    }
    catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << "\n";
    }
}