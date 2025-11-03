#include "LogAnalyzer.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <thread>
#include <algorithm>
#include <stdexcept>

namespace fs = std::filesystem;

LogAnalyzer::LogAnalyzer(const std::string& directoryPath)
: m_directory(directoryPath)
{
    // Keywords exactly as required (with brackets)
    m_keywords = { "[WARN]", "[ERROR]", "[FATAL]", "[INFO]", "[DEBUG]", "[TRACE]" };
    for (const auto &k : m_keywords) m_totals[k] = 0;
}

void LogAnalyzer::processFile(const std::string& filepath)
{
    // Print which file is being processed (do this inside thread)
    {
        std::lock_guard<std::mutex> lg(m_mutex);
        std::cout << "Processing: " << fs::path(filepath).filename().string() << std::endl;
    }

    std::ifstream in(filepath);
    if (!in.is_open()) {
        std::lock_guard<std::mutex> lg(m_mutex);
        std::cerr << "Failed to open: " << filepath << std::endl;
        return;
    }

    // local counts to minimize time holding the global mutex
    std::map<std::string,int> localCounts;
    for (const auto &k : m_keywords) localCounts[k] = 0;

    std::string line;

    // Lambda to count occurrences of a key in a line
    auto countOccurrences = [](const std::string &line, const std::string &key) {
        int cnt = 0;
        size_t pos = 0;
        while ((pos = line.find(key, pos)) != std::string::npos) {
            ++cnt;
            pos += key.size(); // move past this occurrence
        }
        return cnt;
    };

    while (std::getline(in, line)) {
        for (const auto &k : m_keywords) {
            int found = countOccurrences(line, k);
            if (found) localCounts[k] += found;
        }
    }

    // Merge localCounts into global totals safely
    {
        std::lock_guard<std::mutex> lg(m_mutex);
        for (const auto &kv : localCounts) {
            m_totals[kv.first] += kv.second;
        }
        ++m_filesProcessed;
    }
}

void LogAnalyzer::run()
{
    try {
        if (!fs::exists(m_directory) || !fs::is_directory(m_directory)) {
            throw std::runtime_error("Provided path is not a valid directory.");
        }

        std::vector<std::string> logFiles;
        for (const auto &entry : fs::directory_iterator(m_directory)) {
            if (!entry.is_regular_file()) continue;
            if (entry.path().extension() == ".log") {
                logFiles.push_back(entry.path().string());
            }
        }

        std::cout << "Analyzing folder: " << m_directory << std::endl << std::endl;

        // Start one thread per log file
        std::vector<std::thread> threads;
        for (const auto &f : logFiles) {
            threads.emplace_back(&LogAnalyzer::processFile, this, f);
        }

        // join all threads
        for (auto &t : threads) if (t.joinable()) t.join();

        // Lambda to print the summary nicely (strip brackets for display)
        auto printEntry = [](const std::string &key, int count) {
            // display without surrounding brackets if present
            std::string label = key;
            if (!label.empty() && label.front() == '[' && label.back() == ']') {
                label = label.substr(1, label.size() - 2);
            }
            std::cout << label << ": " << count << std::endl;
        };

        std::cout << std::endl << "--- Keyword Summary ---" << std::endl;
        for (const auto &k : m_keywords) {
            printEntry(k, m_totals[k]);
        }
        std::cout << std::endl;
        std::cout << "Analysis complete. Processed " << m_filesProcessed << " file"
                  << (m_filesProcessed == 1 ? "" : "s") << "." << std::endl;

    } catch (const std::exception &ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
    }
}
