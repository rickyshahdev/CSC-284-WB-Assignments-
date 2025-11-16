#ifndef JSON_PROCESSOR_H
#define JSON_PROCESSOR_H

#include <string>
#include <filesystem>
#include <map>

namespace fs = std::filesystem;

class JsonProcessor {
public:
    JsonProcessor();
    size_t processDirectory(const std::string& inputDir, const std::string& outputDir);
    
private:
    void processJsonFile(const fs::path& inputFile, const fs::path& outputFile);
    std::string transformJsonContent(const std::string& content);
    
    std::map<std::string, std::string> replacementMap;
};

#endif // JSON_PROCESSOR_H