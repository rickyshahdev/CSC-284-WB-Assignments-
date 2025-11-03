#include "LogAnalyzer.h"
#include <iostream>
#include <filesystem>
#include <thread>
#include <vector>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <log_directory>\n";
        return 1;
    }

    std::string folderPath = argv[1];
    LogAnalyzer analyzer;

    std::vector<std::string> keywords = {
        "[WARN]", "[ERROR]", "[FATAL]", "[INFO]", "[DEBUG]", "[TRACE]"
    };

    std::vector<std::thread> threads;

    try {
        if (!fs::exists(folderPath) || !fs::is_directory(folderPath)) {
            std::cerr << "Provided path is not a valid directory.\n";
            return 1;
        }

        std::cout << "Analyzing folder: " << folderPath << "\n\n";

        for (const auto& entry : fs::directory_iterator(folderPath)) {
            if (entry.is_regular_file() && entry.path().extension() == ".log") {
                threads.emplace_back(&LogAnalyzer::analyzeFile, &analyzer,
                                     entry.path().string(), keywords);
            }
        }

        // Join all threads
        for (auto& t : threads) {
            if (t.joinable())
                t.join();
        }

        analyzer.printSummary();

        std::cout << "\nAnalysis complete. Processed "
                  << threads.size() << " file"
                  << (threads.size() == 1 ? "" : "s") << ".\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
