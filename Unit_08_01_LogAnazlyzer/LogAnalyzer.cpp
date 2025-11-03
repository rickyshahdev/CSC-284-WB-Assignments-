#include "LogAnalyzer.h"
#include <fstream>
#include <iostream>
#include <algorithm>

// Function to analyze a single log file
void LogAnalyzer::analyzeFile(const std::string& filename, const std::vector<std::string>& keywords) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error: Could not open file " << filename << "\n";
        return;
    }

    std::cout << "Processing: " << filename << std::endl;
    std::string line;

    while (std::getline(file, line)) {
        for (const auto& keyword : keywords) {
            if (line.find(keyword) != std::string::npos) {
                // Thread-safe increment of keyword count
                std::lock_guard<std::mutex> lock(countMutex);
                keywordCounts[keyword]++;
            }
        }
    }
}

// Function to print summary using std::for_each and a lambda
void LogAnalyzer::printSummary() const {
    std::cout << "\n--- Keyword Summary ---\n";

    std::lock_guard<std::mutex> lock(countMutex);

    std::for_each(keywordCounts.begin(), keywordCounts.end(),
                  [](const std::pair<std::string, int>& entry) {
                      std::string key = entry.first;
                      // remove brackets for cleaner display
                      if (!key.empty() && key.front() == '[' && key.back() == ']')
                          key = key.substr(1, key.size() - 2);
                      std::cout << key << ": " << entry.second << "\n";
                  });

    std::cout << "-----------------------\n";
}
