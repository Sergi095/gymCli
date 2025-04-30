#include "Utils.h"
#include <ctime>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <iomanip>



// Add these improved date functions to your Utils.cpp file

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
    // Parse the date string (format: YYYY-MM-DD)
    int year, month, day;
    if (sscanf(dateStr.c_str(), "%d-%d-%d", &year, &month, &day) != 3) {
        return "Unknown"; // Failed to parse date
    }
    
    // Use Zeller's Congruence formula to calculate day of week
    if (month < 3) {
        month += 12;
        year--;
    }
    
    int h = (day + (13 * (month + 1)) / 5 + year + year / 4 - year / 100 + year / 400) % 7;
    
    // Convert h to day of week (0 = Saturday, 1 = Sunday, ..., 6 = Friday)
    const std::string days[] = {"Saturday", "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday"};
    return days[h];
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
