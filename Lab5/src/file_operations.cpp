#include "file_operations.h"
#include <iostream>
#include <iomanip>
#include <cstring>
#include <cstdio>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>

// ── listFiles ─────────────────────────────────────────────────────────────────
void FileOperations::listFiles(const std::string& path, bool showHidden) {
    std::cout << "\nContents of " << path << ":\n";
    std::cout << "--------------------------------\n";

    DIR* dir = opendir(path.c_str());
    if (!dir) {
        std::cerr << "Error: Cannot open directory: " << path << std::endl;
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // Skip hidden files unless requested
        if (!showHidden && entry->d_name[0] == '.')
            continue;

        std::string fullPath = path + "/" + entry->d_name;
        struct stat st;
        long size = 0;
        bool isDir = false;

        if (stat(fullPath.c_str(), &st) == 0) {
            size  = st.st_size;
            isDir = S_ISDIR(st.st_mode);
        }

        const char* type = isDir ? "[DIR] " : "[FILE]";
        std::cout << std::left  << std::setw(8)  << type
                  << std::setw(36) << entry->d_name
                  << std::right << std::setw(12) << size << " bytes\n";
    }
    closedir(dir);
}

// ── getFileSize ───────────────────────────────────────────────────────────────
long FileOperations::getFileSize(const std::string& path) {
    struct stat st;
    if (stat(path.c_str(), &st) == 0)
        return st.st_size;
    return -1;
}

// ── fileExists ────────────────────────────────────────────────────────────────
bool FileOperations::fileExists(const std::string& path) {
    struct stat st;
    return (stat(path.c_str(), &st) == 0);
}

// ── createDirectory ───────────────────────────────────────────────────────────
bool FileOperations::createDirectory(const std::string& path) {
    // 0755 = owner rwx, group rx, others rx
    return (mkdir(path.c_str(), 0755) == 0);
}

// ── deleteFile ────────────────────────────────────────────────────────────────
bool FileOperations::deleteFile(const std::string& path) {
    return (remove(path.c_str()) == 0);
}

// ── renameFile ────────────────────────────────────────────────────────────────
bool FileOperations::renameFile(const std::string& oldPath, const std::string& newPath) {
    return (rename(oldPath.c_str(), newPath.c_str()) == 0);
}

// ── collectFiles (recursive) ──────────────────────────────────────────────────
std::vector<std::string> FileOperations::collectFiles(const std::string& directory) {
    std::vector<std::string> files;

    DIR* dir = opendir(directory.c_str());
    if (!dir) return files;

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        std::string fullPath = directory + "/" + entry->d_name;
        struct stat st;

        if (stat(fullPath.c_str(), &st) != 0) continue;

        if (S_ISDIR(st.st_mode)) {
            // Recurse into subdirectory
            std::vector<std::string> sub = collectFiles(fullPath);
            files.insert(files.end(), sub.begin(), sub.end());
        } else {
            files.push_back(fullPath);
        }
    }
    closedir(dir);
    return files;
}

// ── findFilesByExtension ──────────────────────────────────────────────────────
std::vector<std::string> FileOperations::findFilesByExtension(
        const std::string& directory, const std::string& extension) {

    std::vector<std::string> allFiles = collectFiles(directory);
    std::vector<std::string> result;

    for (size_t i = 0; i < allFiles.size(); i++) {
        const std::string& f = allFiles[i];
        if (f.length() >= extension.length()) {
            if (f.substr(f.length() - extension.length()) == extension) {
                result.push_back(f);
            }
        }
    }
    return result;
}
