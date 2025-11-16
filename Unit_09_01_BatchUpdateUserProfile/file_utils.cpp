#include "file_utils.h"
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

std::string FileUtils::createOutputDirectory() {
    std::string timestamp = getCurrentTimestamp();
    std::string dirName = "user_profiles_updated_" + timestamp;
    
    // Create the directory
    fs::create_directory(dirName);
    
    return dirName;
}

std::string FileUtils::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::tm tm;
#ifdef _WIN32
    localtime_s(&tm, &time_t);
#else
    localtime_r(&time_t, &tm);
#endif
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y%m%d_%H%M%S");
    
    return oss.str();
}