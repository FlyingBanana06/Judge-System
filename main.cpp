// main.cpp

#include <iostream>
#include <string>
#include "Judge.hpp"
#include "ColorPrint.hpp"
#include "Utils.hpp"


namespace {
    const std::string userDataPath     = "./data/user/user.csv";
    const std::string problemDataPath  = "./data/problem/problem.csv";
    const std::string version            = "4.4";
}

int main() {
    ClearScreen();

    try {
        JudgeSystem judge(userDataPath, problemDataPath, version);
        std::cout << blue("Simple Judge System started! Version: ") << version << "\n";
        judge.loginProcess();
    } catch (const std::exception& e) {
        std::cerr << red("[Fatal Error] ") << e.what() << '\n';
        return 1;
    }
    
    return 0;
}
