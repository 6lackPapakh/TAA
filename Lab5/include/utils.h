#pragma once
#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

class Utils {
public:
    static void printColor(const std::string& text, const std::string& color);
    static std::string getCurrentTime();
    static std::vector<std::string> split(const std::string& str, char delimiter);
    static void clearScreen();
    static void pressAnyKey();
    static std::string center(const std::string& text, int width);
    static std::string getUserInput(const std::string& prompt);
    static std::string getPassword(const std::string& prompt);
};

#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_RESET   "\x1b[0m"

#endif
