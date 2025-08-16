// Problem.cpp

#include "Problem.hpp"
#include "ColorPrint.hpp"
#include "Utils.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <cstdlib>
#include <algorithm>
#include <random>
#include <limits>

#ifdef _WIN32
#define COMPILE_CMD "g++ %s -o build/user_program.exe"
#define RUN_CMD ".\\build\\user_program.exe < %s > data/user/user_output.txt"
#else
#define COMPILE_CMD "g++ %s -o build/user_program"
#define RUN_CMD "./build/user_program < %s > data/user/user_output.txt"
#endif

namespace fs = std::filesystem;

Problem::Problem(std::string t, std::string b)
    : title(std::move(t)), basePath(std::move(b)) {}


// --- Internal File Helpers ---
namespace {
    // 去除字串前後的空白字元
    std::string trimStr(const std::string& s) {
        auto start = s.find_first_not_of(" \t\r\n");
        auto end   = s.find_last_not_of(" \t\r\n");
        return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
    }

    // 從 CSV 檔案讀取題目資料，並將其存入 problemList
    void loadProblems(const std::string& csvPath, std::vector<Problem>& problems) {
        problems.clear();

        std::ifstream file(csvPath);
        if (!file.is_open()) {
            std::cerr << red("Error: Cannot open problem data file: ") << csvPath << "\n";
            exit(1);
        }

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            std::stringstream ss(line);
            std::string title, relativePath;
            std::getline(ss, title, ',');
            std::getline(ss, relativePath, ',');

            title = trimStr(title);
            relativePath = trimStr(relativePath);
            if (title.empty() || relativePath.empty()) continue;

            fs::path basePath = relativePath;
            if (!fs::exists(basePath / "testcases")) {
                std::cerr << yellow("Warning: testcases directory not found for ") << title
                        << " (" << (basePath / "testcases").string() << ")\n";
            }

            problems.emplace_back(title, basePath.string());
        }
    }

    // 將新題目寫入 CSV 檔案
    void appendProblemToCSV(const std::string& csvPath, const std::string& title, const std::string& base) {
        std::ofstream out(csvPath, std::ios::app);
        if (!out) {
            std::cerr << red("Warning: cannot open problem data CSV to append: ") << csvPath << "\n";
            return;
        }
        out << title << "," << base << "\n";
        std::cerr << "Writing to: " << fs::absolute(csvPath) << "\n";
    }

    // 持續讀取使用者輸入並寫入檔案，直到輸入"."為止
    void saveInput(const fs::path& filepath) {
        std::ofstream file(filepath);
        std::string line;
        while (true) {
            std::getline(std::cin, line);
            if (line == ".") break;
            file << line << '\n';
        }
    }
}


// --- Internal Utility Helpers ---
namespace {
    void createNewCodeFile() {
        // 建立新 code 檔案
        std::string newFileName;
        std::cout << cyan("Enter new code file name (e.g. newcodefile.cpp): ");
        std::cin >> newFileName;

        fs::path codePath = fs::path("data/user/program") / newFileName;
        fs::create_directories(codePath.parent_path());

        std::cout << "\033[36m" // cyan
                << "Enter your code for " << newFileName
                << " (end with '.' on a single line):\n"
                << "\033[0m";

        saveInput(codePath);

        std::cout << green("File created: ") << codePath << "\n\n";
    }

    // 編譯程式碼，成功回傳 true
    bool compileCode(const std::string& codePath) {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), COMPILE_CMD, codePath.c_str());
        return system(cmd) == 0;
    }

    // 執行程式，使用 input 檔案，回傳是否成功
    bool runCode(const std::string& inputPath) {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), RUN_CMD, inputPath.c_str());
        return system(cmd) == 0;
    }

    // 比對實際輸出與預期輸出，逐行檢查
    bool compareOutput(const std::string& expectedPath, const std::string& actualPath) {
        std::ifstream expected(expectedPath);
        std::ifstream actual(actualPath);
        std::string eLine, aLine;

        while (std::getline(expected, eLine)) {
            if (!std::getline(actual, aLine) || eLine != aLine)
                return false;
        }

        return !std::getline(actual, aLine); // 檢查是否還有額外輸出
    }

    bool compileAndRun(const std::string& codePath, const std::vector<fs::path>& ins) {
        std::cout << yellow("Compiling...\n");
        if (!compileCode(codePath)) {
            std::cerr << red("Compile error.\n");
            return false;
        }
        bool allPassed = true;
        for (size_t i = 0; i < ins.size(); ++i) {
            fs::path inPath = ins[i];
            fs::path expected = inPath;
            expected.replace_extension(".out");

            std::cout << yellow("Running test case ") << (i + 1) << "...\n";
            if (!runCode(inPath.string())) {
                std::cerr << red("Runtime error on test case ") << (i + 1) << "\n";
                allPassed = false;
                break;
            }
            if (!compareOutput(expected.string(), "data/user/user_output.txt")) {
                std::cout << red("Wrong Answer on test case ") << (i + 1) << "\n";
                allPassed = false;
                break;
            } else {
                std::cout << green("Test case ") << (i + 1) << " passed.\n";
            }
        }
        if (allPassed) std::cout << green("Accepted! All test cases passed.\n");
        return allPassed;
    }
}



// --- ProblemSystem methods ---
void ProblemSystem::init(const std::string& problemDataPath) {
    loadProblems(problemDataPath, problemList);
}

bool ProblemSystem::listAllProblems() const {
    if (problemList.empty()) {
        std::cout << red("No problems found.\n");
        return false;
    }
    std::cout << cyan("=== Problem List ===\n");
    for (size_t i = 0; i < problemList.size(); ++i) {
        std::cout << "[" << i + 1 << "] " << problemList[i].getTitle() << "\n";
    }
    return true;
}

bool ProblemSystem::printProblemDescription(int id) const {
    if (id < 1 || id > (int)problemList.size()) return false;
    fs::path descPath = fs::path(problemList[id - 1].getBasePath()) / "description.txt";

    std::cout << green("\n=== Description ===\n");
    std::ifstream desc(descPath);
    if (desc) {
        std::string line;
        while (std::getline(desc, line)) {
            std::cout << line << '\n';
        }
    } else {
        std::cout << red("Description file not found: ") << descPath.string() << '\n';
    }
    std::cout << green("====================\n");
    return true;
}

int ProblemSystem::randomProblem() const {
    if (problemList.empty()) {
        std::cout << red("No problems available.\n");
        return -1;
    }
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, (int)problemList.size() - 1);
    int idx = dist(rng);

    std::cout << yellow("Random Problem: ") << problemList[idx].getTitle() << "\n";
    printProblemDescription(idx + 1);
    return idx;
}

// 新增一個新的題目集
void ProblemSystem::newProblemSet(const std::string& problemDataPath) {
    // 輸入題目標題
    std::string title;
    std::cout << yellow("Enter problem title: ");
    std::getline(std::cin >> std::ws, title);
    if (title.empty()) {
        std::cout << red("Title cannot be empty.\n");
        return;
    }

    std::string folderName = title;
    std::replace(folderName.begin(), folderName.end(), ' ', '-'); // 將空格替換為 '-'

    fs::path base = fs::path("data/problem") / folderName;
    fs::path testdir = base / "testcases";

    // 創建資料夾
    try {
        fs::create_directories(testdir);
    } catch (...) {
        std::cout << red("Failed to create problem directory: ") << base.string() << '\n';
        return;
    }

    // 輸入題目敘述
    std::cout << yellow("Enter problem description (end with '.' on a single line):\n");
    saveInput(base / "description.txt");

    // 輸入測資
    int cases = 0;
    std::cout << cyan("How many test cases? ");
    std::cin >> cases;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    for (int i = 1; i <= cases; ++i) {
        std::cout << cyan("Input for test case #") << i << cyan(" (end with '.'):\n");
        saveInput(testdir / (std::to_string(i) + ".in"));

        std::cout << cyan("Expected output for test case #") << i << cyan(" (end with '.'):\n");
        saveInput(testdir / (std::to_string(i) + ".out"));
    }

    // 新增題目到 problemList，將資料寫入 problem.csv
    addProblem(Problem(title, base.string()));
    appendProblemToCSV(problemDataPath, title, base.string());
    std::cout << green("Problem added: ") << title << '\n';
}

void ProblemSystem::addProblem(const Problem& p) {
    problemList.push_back(p);
}

std::vector<fs::path> ProblemSystem::testcasePrepare(const int id) {
    if (id < 1 || id > (int)problemList.size()) {
        std::cout << red("Invalid problem id.\n");
        return {};
    }

    const auto& prob = problemList[id - 1];
    fs::path tcDir = fs::path(prob.getBasePath()) / "testcases";
    if (!fs::exists(tcDir) || !fs::is_directory(tcDir)) {
        std::cerr << red("Testcases directory missing: ") << tcDir.string() << '\n';
        return {};
    }

    std::vector<fs::path> ins;
    for (auto &entry : fs::directory_iterator(tcDir)) {
        if (entry.path().extension() == ".in") ins.push_back(entry.path());
    }
    if (ins.empty()) {
        std::cerr << red("No .in testcases found.\n");
        return {};
    }
    std::sort(ins.begin(), ins.end());
    return ins;
}


void ProblemSystem::submitCode(const int id) {
    // 檢查測資，若準備失敗則終止流程
    auto ins = testcasePrepare(id);
    if (ins.empty()) return;

    std::string codePath;
    while (true) {
        std::string input;
        while (true) {
            std::cout << blue("\nEnter your code file name (e.g. mysolution.cpp)\n"
                              "Or enter -1 to create a new code file: ");
            std::cin >> input;
            if (input != "-1") break; // 使用者輸入的是現有檔案名，直接結束詢問
            createNewCodeFile();
        }

        codePath = "data/user/program/" + input;
        if (compileAndRun(codePath, ins)) break; // 若成功通過測資，則結束流程

        std::cout << yellow("\nRetry? (y/n): ");
        if (!promptYesNo()) {
            ClearScreen();
            break;
        }
    }
}
