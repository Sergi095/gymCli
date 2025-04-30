#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

std::string getCurrentDate();
std::string getCurrentDayOfWeek();
std::string getDayOfWeek(const std::string& dateStr); // Format: YYYY-MM-DD
std::string toLower(const std::string& str);
bool containsIgnoreCase(const std::string& str, const std::string& substr);
bool equalsIgnoreCase(const std::string& str1, const std::string& str2);
std::vector<std::string> getUpperBodyCategories();
std::vector<std::string> getLowerBodyCategories();
bool isUpperBodyCategory(const std::string& category);
bool isLowerBodyCategory(const std::string& category);

#endif // UTILS_H
