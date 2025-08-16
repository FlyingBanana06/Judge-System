// Judge.hpp

#ifndef JUDGE_HPP
#define JUDGE_HPP

#include <string>
#include "Account.hpp"
#include "Problem.hpp"

class JudgeSystem {
private:
    AccountSystem accountSystem;
    ProblemSystem problemSystem;

    std::string userDataPath;
    std::string problemDataPath;
    std::string version;
    std::string status;

public:
    JudgeSystem() = default;
    JudgeSystem(const std::string& userPath,
                const std::string& problemPath,
                const std::string& version);

    void loadData();
    void loginProcess();
    bool mainPageProcess();

    std::string getUserPath() const { return userDataPath; }
    std::string getProblemPath() const { return problemDataPath; }
    std::string getVersion() const { return version; }
};

#endif // JUDGE_HPP