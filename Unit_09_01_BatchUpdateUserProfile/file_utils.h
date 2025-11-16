#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <string>

class FileUtils {
public:
    static std::string createOutputDirectory();
    static std::string getCurrentTimestamp();
};

#endif // FILE_UTILS_H