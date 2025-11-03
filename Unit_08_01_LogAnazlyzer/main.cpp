 #include "LogAnalyzer.h"
#include <iostream>

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <log_directory>" << std::endl;
        return 1;
    }

    std::string folder = argv[1];

    try {
        LogAnalyzer analyzer(folder);
        analyzer.run();
    } catch (const std::exception &ex) {
        std::cerr << "Fatal error: " << ex.what() << std::endl;
        return 2;
    }

    return 0;
}
