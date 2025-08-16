// JudgeSystem.cpp

#include "Judge.hpp"
#include "ColorPrint.hpp"
#include "Utils.hpp"

#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
#include <vector>
#include <stdexcept>

// Constructor
JudgeSystem::JudgeSystem(const std::string& userPath,
                         const std::string& problemPath,
                         const std::string& version)
    // 將得到的參數資訊更新到 JudgeSystem 的成員 (member) 中。
    // 將 status 設為 "NOT READY" 表示系統尚未完成任何初始化。
    : userDataPath(userPath),
      problemDataPath(problemPath),
      version(version),
      status("NOT READY") {
}


// --- Internal helpers ---
namespace {
    // 讀取動畫，讓程式看起來好像比較帥。
    void effectLoading(const std::string& content) {
        const char* spinner[] = {"|", "/", "-", "\\"};
        for (int i = 0, j = 0; j < 8; j++) { 
            std::cout << "\033[32m" << content << "\033[0m" << spinner[i] << "\r";
            std::flush(std::cout);
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
            i = (i + 1) % 4;
        }
        return;
    }

    // 輸出登入歡迎訊息
    void printLoginMsg() {
        std::cout << cyan(
            "====================================\n"
            "   Welcome to Simple Judge System   \n"
            "====================================\n"
        );
    }

    // 輸出主選單
    void mainpagePrint() {
        // Top border
        std::cout << "+" << std::string(35, '-') << "+\n";
        std::cout << yellow("Please choose an operation: \n");
        std::cout << "+" << std::string(35, '-') << "+\n";

        // Menu options
        std::cout << green("(1) Current user\n"
                           "(2) Judge system version\n")
                   << blue("(3) Print all problems\n"
                           "(4) Random problem\n"
                           "(5) Submit code\n")
                    << red("(6) Add new problem (admin only)\n")
                 << yellow("(7) Sign out\n"
                           "(8) Exit program\n");

        // Bottom border
        std::cout << "+" << std::string(35, '-') << "+\n";
        std::cout << cyan("User input: ");

        return;
    }
}


// --- ProblemSystem methods ---

// 初始化 AccountSystem 與 ProblemSystem。呼叫這兩個 class 的 init function。
// 呼叫完後透過 try-catch 語法來讀取 ./msg/login.txt 檔案來輸出訊息。
void JudgeSystem::loadData() {
    // Step1: 初始化帳號系統
    accountSystem.init(userDataPath);
    effectLoading("Status - Loading user data...");
    std::cout << green("Status - Loading user data...OK!\n");

    // Step2: 初始化題目系統
    problemSystem.init(problemDataPath);
    effectLoading("Status - Loading problem data...");
    std::cout << green("Status - Loading problem data...OK!\n");

    // Step3: 歡迎使用者
    printLoginMsg();
}

// 系統狀態分成以下三種：未初始化 (NOT READY)、使用者未登入 (USER LOGIN)、使用者已登入 (READY)。
// 不同狀態下將程式導向對應的 Function。
void JudgeSystem::loginProcess() {
    while (status != "READY") {
        if (status == "NOT READY") {
            loadData();
            status = "USER LOGIN";
            continue;
        }
        else if (status == "USER LOGIN") {
            std::pair<bool, std::string> user_info = accountSystem.login();
            if (!user_info.first) continue;
            status = "READY";
            break; // 登入成功
        }
    }
    // status == "READY"
    while (mainPageProcess()); // 呼叫主選單
}

// 主選單：負責顯示選單、讀取使用者輸入並執行對應功能。
// 功能：顯示使用者資訊、版本號、題目列表、隨機題目、提交程式碼、新增題目、結束程式等。
bool JudgeSystem::mainPageProcess() {
    mainpagePrint();
    std::string input;
    std::cin >> input;
    ClearScreen();
    std::cout << blue("User input: ") << input << '\n';

    int opt;
    if (input.length() == 1 && input[0] >= '1' && input[0] <= '8') {
        opt = input[0] - '0'; // 轉為 int
    } else {
        std::cout << red("Invalid input. Please enter a number between 1 and 8.\n");
        return true; // 回到主選單
    }

    switch (opt) {
        case 1: {
            std::cout << green("Username: ") << accountSystem.getuserLogin() << '\n';
            break;
        }
        case 2: {
            std::cout << green("VERSION: ") << version << '\n';
            break;
        }
        case 3: {
            // 列出並讓使用者選題目
            // 如果沒有題目，則結束流程。
            if (!problemSystem.listAllProblems()) break;

            std::cout << cyan("Do you want to select a problem to solve? (y/n): ");
            if (!promptYesNo()) {
                ClearScreen();
                break;
            }
            int problemId;

            while (true) {
                std::cout << yellow("Please input the problem ID: ");
                std::cin >> problemId;

                if (problemId < 1 || problemId > (int)problemSystem.getProblemList().size()) {
                    std::cout << red("Invalid problem ID.\n");
                    continue;
                }
                break;
            }

            // 顯示題目說明並提交判題
            problemSystem.printProblemDescription(problemId);
            problemSystem.submitCode(problemId);            
            break;
        }
        case 4: {
            // 隨機題目
            int idx = problemSystem.randomProblem(); // returns -1 if no problem
            if (idx == -1) break;

            std::cout << yellow("Do you want to solve this problem? (y/n): ");
            if (!promptYesNo()) {
                ClearScreen();
                break;
            }
            
            problemSystem.submitCode(idx + 1); // submitCode uses 1-based id
            break;
        }
        case 5: {
            // 單獨提交程式碼（無題目說明）
            if (!problemSystem.listAllProblems()) break;

            while (true) {
                int problemId;
                std::cout << yellow("Please input the problem ID to submit: ");
                std::cin >> problemId;

                if (problemId < 1 || problemId > (int)problemSystem.getProblemList().size()) {
                    std::cout << red("Invalid problem ID.\n");
                    continue;
                }

                problemSystem.submitCode(problemId);
                break;
            }
            break;
        }
        case 6: {
            if (accountSystem.getuserLogin() == "admin") {
                problemSystem.newProblemSet(problemDataPath);
            } else {
                std::cout << red("Only admin can add new problem!\n");
            }
            break;
        }
        case 7: {
            std::cout << yellow("User logged out!!\n\n");
            status = "USER LOGIN";
            loginProcess(); // 回到登入流程
            return false; // exit
        }
        case 8: {
            ClearScreen();
            std::cout << cyan(
                "==========================\n"
                "   Thanks for visiting!   \n"
                "    See you next time!    \n"
                "==========================\n"
            );
            return false; // exit
        }
        default: {
            std::cout << red("Invalid option. Please try again.\n");
            break;
        }
    }

    return true;
}