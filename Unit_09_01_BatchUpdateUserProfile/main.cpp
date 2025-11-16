#include <iostream>
#include <string>
#include <filesystem>
#include "json_processor.h"
#include "file_utils.h"

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_user_profiles_directory>" << std::endl;
        return 1;
    }

    std::string inputPath = argv[1];
    
    if (!fs::exists(inputPath) || !fs::is_directory(inputPath)) {
        std::cerr << "Error: Directory '" << inputPath << "' does not exist or is not a directory." << std::endl;
        return 1;
    }

    try {
        // Create output directory with timestamp
        std::string outputDir = FileUtils::createOutputDirectory();
        std::cout << "Output directory: " << outputDir << std::endl;

        // Process all JSON files
        JsonProcessor processor;
        size_t processedFiles = processor.processDirectory(inputPath, outputDir);
        
        std::cout << "Successfully processed " << processedFiles << " files." << std::endl;
        std::cout << "Output saved to: " << outputDir << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}