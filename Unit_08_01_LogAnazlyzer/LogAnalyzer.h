#ifndef LOGANALYZER_H
#define LOGANALYZER_H

#include <string>
#include <map>
#include <vector>
#include <mutex>

class LogAnalyzer {
public:
    explicit LogAnalyzer(const std::string& directoryPath);
    void run();

private:
    std::string m_directory;
    std::vector<std::string> m_keywords;
    std::map<std::string,int> m_totals;
    std::mutex m_mutex;
    int m_filesProcessed = 0;

    void processFile(const std::string& filepath);
};

#endif // LOGANALYZER_H
