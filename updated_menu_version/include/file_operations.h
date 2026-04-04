#pragma once
#ifndef FILE_OPERATIONS_H
#define FILE_OPERATIONS_H

#include <string>
#include <vector>

class FileOperations {
public:
    static void listFiles(const std::string& path, bool showHidden = false);
    static long getFileSize(const std::string& path);
    static bool fileExists(const std::string& path);
    static bool createDirectory(const std::string& path);
    static bool deleteFile(const std::string& path);
    static bool renameFile(const std::string& oldPath, const std::string& newPath);
    static std::vector<std::string> collectFiles(const std::string& directory);
    static std::vector<std::string> findFilesByExtension(const std::string& directory,
                                                          const std::string& extension);
};

#endif
