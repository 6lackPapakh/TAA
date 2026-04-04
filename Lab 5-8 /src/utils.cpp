#include "utils.h"
#include <iostream>
#include <sstream>
#include <ctime>
#include <cstdio>
#include <termios.h>   // Linux terminal control
#include <unistd.h>

// ── printColor ────────────────────────────────────────────────────────────────
// On Linux, ANSI escape codes work natively in most terminals
void Utils::printColor(const std::string& text, const std::string& color) {
    std::cout << color << text << COLOR_RESET;
}

// ── getCurrentTime ────────────────────────────────────────────────────────────
std::string Utils::getCurrentTime() {
    time_t now = time(0);
    struct tm* ts = localtime(&now);
    char buf[80];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ts);
    return std::string(buf);
}

// ── split ─────────────────────────────────────────────────────────────────────
std::vector<std::string> Utils::split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter))
        tokens.push_back(token);
    return tokens;
}

// ── clearScreen ───────────────────────────────────────────────────────────────
void Utils::clearScreen() {
    // ANSI escape: clear screen and move cursor to top-left
    std::cout << "\033[2J\033[1;1H";
}

// ── pressAnyKey ───────────────────────────────────────────────────────────────
// Reads one raw keypress without echoing it, replacing Windows _getch()
void Utils::pressAnyKey() {
    std::cout << "\nPress any key to continue...";
    std::cout.flush();

    // Save current terminal settings
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    // Disable buffering and echo
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    getchar();  // read one character without Enter

    // Restore terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    std::cout << std::endl;
}

// ── center ────────────────────────────────────────────────────────────────────
std::string Utils::center(const std::string& text, int width) {
    size_t len = text.length();
    if (width <= (int)len) return text;
    size_t left  = (width - len) / 2;
    size_t right = width - len - left;
    return std::string(left, ' ') + text + std::string(right, ' ');
}

// ── getUserInput ──────────────────────────────────────────────────────────────
std::string Utils::getUserInput(const std::string& prompt) {
    std::cout << prompt;
    std::string input;
    std::getline(std::cin, input);
    return input;
}

// ── getPassword ───────────────────────────────────────────────────────────────
// Reads a password character by character without echoing it.
// Replaces Windows _getch() with POSIX termios raw mode.
std::string Utils::getPassword(const std::string& prompt) {
    std::cout << prompt;
    std::cout.flush();

    std::string password;

    // Switch terminal to raw mode: no echo, no line buffering
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);   // disable canonical mode and echo
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    char ch;
    while (read(STDIN_FILENO, &ch, 1) == 1) {
        if (ch == '\n' || ch == '\r') {
            // Enter pressed — done
            break;
        } else if (ch == 127 || ch == '\b') {
            // Backspace / Delete
            if (!password.empty()) {
                std::cout << "\b \b";   // erase the * on screen
                std::cout.flush();
                password.pop_back();
            }
        } else if (ch >= 32 && ch < 127) {
            // Printable character
            password.push_back(ch);
            std::cout << '*';
            std::cout.flush();
        }
    }

    // Restore original terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    std::cout << std::endl;

    return password;
}
