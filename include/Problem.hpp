// Problem.hpp

#ifndef PROBLEM_HPP
#define PROBLEM_HPP
    
#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

class Problem {
private:
    std::string title;      // e.g., "Problem Title"
    std::string basePath;   // e.g., "problem/problem-name"

public:
    Problem(std::string t, std::string b);
    std::string getTitle() const { return title; };
    std::string getBasePath() const { return basePath; }
};

class ProblemSystem {
    friend class JudgeSystem;
private:
    std::vector<Problem> problemList;

public:
    void init(const std::string& problemDataPath);
    bool listAllProblems() const;
    bool printProblemDescription(int id) const;
    int randomProblem() const;
    void submitCode(const int id);
    void addProblem(const Problem& p);
    void newProblemSet(const std::string& problemDataPath);
    std::vector<fs::path> testcasePrepare(const int id);
    const std::vector<Problem>& getProblemList() const { return problemList; };
};

#endif // PROBLEM_HPP