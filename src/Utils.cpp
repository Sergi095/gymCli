#include "Utils.h"
#include <ctime>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstdio>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

namespace {

const std::vector<std::string>& daysOfWeek() {
    static const std::vector<std::string> days = {
        "Monday", "Tuesday", "Wednesday", "Thursday",
        "Friday", "Saturday", "Sunday"
    };
    return days;
}

} // namespace

std::string getCurrentDate() {
    time_t now = time(0);
    struct tm timeinfo;

#ifdef _WIN32
    localtime_s(&timeinfo, &now);
#else
    timeinfo = *localtime(&now);
#endif

    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", &timeinfo);
    return std::string(buffer);
}

std::string getCurrentDayOfWeek() {
    time_t now = time(0);
    struct tm timeinfo;

#ifdef _WIN32
    localtime_s(&timeinfo, &now);
#else
    timeinfo = *localtime(&now);
#endif

    char buffer[80];
    strftime(buffer, sizeof(buffer), "%A", &timeinfo); // %A gives the full weekday name
    return std::string(buffer);
}

std::string getDayOfWeek(const std::string& dateStr) {
    if (!isValidDate(dateStr)) {
        return "Unknown";
    }

    int year = 0;
    int month = 0;
    int day = 0;
    std::sscanf(dateStr.c_str(), "%d-%d-%d", &year, &month, &day);

    std::tm date = {};
    date.tm_year = year - 1900;
    date.tm_mon = month - 1;
    date.tm_mday = day;
    date.tm_isdst = -1;
    std::mktime(&date);

    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%A", &date);
    return std::string(buffer);
}

bool isValidDate(const std::string& dateStr) {
    if (dateStr.size() != 10 || dateStr[4] != '-' || dateStr[7] != '-') {
        return false;
    }

    int year = 0;
    int month = 0;
    int day = 0;
    char trailing = '\0';
    if (std::sscanf(dateStr.c_str(), "%d-%d-%d%c", &year, &month, &day, &trailing) != 3 ||
        year < 1900 || month < 1 || month > 12 || day < 1 || day > 31) {
        return false;
    }

    std::tm date = {};
    date.tm_year = year - 1900;
    date.tm_mon = month - 1;
    date.tm_mday = day;
    date.tm_isdst = -1;
    if (std::mktime(&date) == static_cast<std::time_t>(-1)) {
        return false;
    }

    return date.tm_year == year - 1900 && date.tm_mon == month - 1 && date.tm_mday == day;
}

std::string normalizeDayOfWeek(const std::string& value) {
    const std::string cleaned = trim(value);
    for (const auto& day : daysOfWeek()) {
        if (equalsIgnoreCase(cleaned, day) ||
            (cleaned.size() == 3 && equalsIgnoreCase(cleaned, day.substr(0, 3)))) {
            return day;
        }
    }
    return "";
}

std::string getDayForDateOrLegacy(const std::string& value) {
    const std::string legacyDay = normalizeDayOfWeek(value);
    return legacyDay.empty() ? getDayOfWeek(value) : legacyDay;
}

std::string trim(const std::string& value) {
    const std::string whitespace = " \t\n\r";
    const size_t start = value.find_first_not_of(whitespace);
    if (start == std::string::npos) {
        return "";
    }
    const size_t end = value.find_last_not_of(whitespace);
    return value.substr(start, end - start + 1);
}

std::string toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

bool containsIgnoreCase(const std::string& str, const std::string& substr) {
    std::string lowerStr = toLower(str);
    std::string lowerSubstr = toLower(substr);
    return lowerStr.find(lowerSubstr) != std::string::npos;
}

bool equalsIgnoreCase(const std::string& str1, const std::string& str2) {
    return toLower(str1) == toLower(str2);
}

int getTerminalWidth() {
    int width = 0;

#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info)) {
        width = info.srWindow.Right - info.srWindow.Left + 1;
    }
#else
    struct winsize size = {};
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) == 0) {
        width = size.ws_col;
    }
#endif

    if (width <= 0) {
        const char* columns = std::getenv("COLUMNS");
        if (columns != nullptr) {
            try {
                width = std::stoi(columns);
            } catch (...) {
                width = 0;
            }
        }
    }

    if (width <= 0) {
        width = 80;
    }
    return std::max(40, std::min(width, 140));
}

bool isInteractiveInput() {
#ifdef _WIN32
    return _isatty(_fileno(stdin)) != 0;
#else
    return isatty(STDIN_FILENO) != 0;
#endif
}

void clearTerminal() {
    if (!isInteractiveInput()) {
        return;
    }

#ifdef _WIN32
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (!GetConsoleScreenBufferInfo(output, &info)) {
        return;
    }
    const DWORD cells = info.dwSize.X * info.dwSize.Y;
    DWORD written = 0;
    const COORD home = {0, 0};
    FillConsoleOutputCharacter(output, ' ', cells, home, &written);
    FillConsoleOutputAttribute(output, info.wAttributes, cells, home, &written);
    SetConsoleCursorPosition(output, home);
#else
    const char* term = std::getenv("TERM");
    if (term != nullptr && std::string(term) != "dumb") {
        std::cout << "\033[2J\033[H";
    }
#endif
}

std::vector<std::string> getUpperBodyCategories() {
    return {
        "chest", "back", "shoulders", "arms", "biceps",
        "triceps", "forearms", "abs", "core", "upper body"
    };
}

std::vector<std::string> getLowerBodyCategories() {
    return {
        "legs", "quads", "hamstrings", "calves", "glutes",
        "lower body", "hips", "thighs"
    };
}

bool isUpperBodyCategory(const std::string& category) {
    std::string lowerCategory = toLower(category);
    std::vector<std::string> upperCategories = getUpperBodyCategories();

    for (const auto& cat : upperCategories) {
        if (containsIgnoreCase(lowerCategory, cat)) {
            return true;
        }
    }

    return false;
}

bool isLowerBodyCategory(const std::string& category) {
    std::string lowerCategory = toLower(category);
    std::vector<std::string> lowerCategories = getLowerBodyCategories();

    for (const auto& cat : lowerCategories) {
        if (containsIgnoreCase(lowerCategory, cat)) {
            return true;
        }
    }

    return false;
}
