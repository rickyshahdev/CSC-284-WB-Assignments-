#include "json_processor.h"
#include <iostream>
#include <fstream>
#include <regex>
#include <sstream>

JsonProcessor::JsonProcessor() {
    // Initialize replacement map with JSON objects as strings
    replacementMap["\"enabled\""] = R"({"status": "enabled", "since": "2024-10-01"})";
    replacementMap["\"disabled\""] = R"({"status": "disabled", "since": "2024-10-01"})";
    replacementMap["\"manage_users\""] = R"({"permission": "manage_users", "granted_at": "2024-10-05", "level": "full"})";
    replacementMap["\"view_content\""] = R"({"permission": "view_content", "granted_at": "2024-09-25", "level": "read-only"})";
}

size_t JsonProcessor::processDirectory(const std::string& inputDir, const std::string& outputDir) {
    size_t processedCount = 0;
    
    try {
        // Create output directory structure and process files
        for (const auto& entry : fs::recursive_directory_iterator(inputDir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                // Calculate relative path and create corresponding output path
                fs::path relativePath = fs::relative(entry.path(), inputDir);
                fs::path outputFile = fs::path(outputDir) / relativePath;
                
                // Create output directory if it doesn't exist
                fs::create_directories(outputFile.parent_path());
                
                // Process the JSON file
                processJsonFile(entry.path(), outputFile);
                processedCount++;
                
                std::cout << "Processed: " << entry.path() << " -> " << outputFile << std::endl;
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
        throw;
    }
    
    return processedCount;
}

void JsonProcessor::processJsonFile(const fs::path& inputFile, const fs::path& outputFile) {
    try {
        // Read entire file content
        std::ifstream inputStream(inputFile);
        if (!inputStream.is_open()) {
            throw std::runtime_error("Cannot open file: " + inputFile.string());
        }
        
        std::stringstream buffer;
        buffer << inputStream.rdbuf();
        std::string content = buffer.str();
        inputStream.close();
        
        // Transform JSON content
        std::string transformedContent = transformJsonContent(content);
        
        // Write transformed content to output file
        std::ofstream outputStream(outputFile);
        if (!outputStream.is_open()) {
            throw std::runtime_error("Cannot create output file: " + outputFile.string());
        }
        
        outputStream << transformedContent;
        outputStream.close();
        
    } catch (const std::exception& e) {
        std::cerr << "Error processing file " << inputFile << ": " << e.what() << std::endl;
        
        // Copy file as-is if processing fails
        try {
            fs::copy_file(inputFile, outputFile, fs::copy_options::overwrite_existing);
            std::cout << "Copied (unprocessed): " << inputFile << std::endl;
        } catch (const fs::filesystem_error& copyError) {
            std::cerr << "Failed to copy file " << inputFile << ": " << copyError.what() << std::endl;
        }
    }
}

std::string JsonProcessor::transformJsonContent(const std::string& content) {
    std::string result = content;
    
    // Replace email domains
    std::regex emailRegex("@company\\.com");
    result = std::regex_replace(result, emailRegex, "@newcompany.com");
    
    // Replace values using the replacement map
    for (const auto& [oldValue, newValue] : replacementMap) {
        size_t pos = 0;
        while ((pos = result.find(oldValue, pos)) != std::string::npos) {
            // Check if this is a standalone value (not part of a larger string)
            bool isStandalone = true;
            if (pos > 0) {
                char prevChar = result[pos - 1];
                if (std::isalnum(prevChar) || prevChar == '_' || prevChar == '"') {
                    isStandalone = false;
                }
            }
            if (pos + oldValue.length() < result.length()) {
                char nextChar = result[pos + oldValue.length()];
                if (std::isalnum(nextChar) || nextChar == '_' || nextChar == '"') {
                    isStandalone = false;
                }
            }
            
            if (isStandalone) {
                result.replace(pos, oldValue.length(), newValue);
                pos += newValue.length();
            } else {
                pos += oldValue.length();
            }
        }
    }
    
    return result;
}