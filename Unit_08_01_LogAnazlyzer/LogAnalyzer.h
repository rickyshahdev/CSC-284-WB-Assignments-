#ifndef LOG_ANALYZER_H
#define LOG_ANALYZER_H

#include <map>
#include <string>
#include <vector>
#include <mutex>

class LogAnalyzer {
private:
    std::map<std::string, int> keywordCounts;
    mutable std::mutex countMutex; // mutable because we'll lock in const function

public:
    void analyzeFile(const std::string& filename, const std::vector<std::string>& keywords);
    void printSummary() const;
};

#endif
