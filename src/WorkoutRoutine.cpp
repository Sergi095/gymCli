#include "WorkoutRoutine.h"
#include "Utils.h"
#include <sstream>
#include <algorithm>

WorkoutRoutine::WorkoutRoutine(const std::string& routineName) : name(routineName) {
    // Initialize with default values for all days
    std::vector<std::string> days = {
        "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"
    };
    
    for (const auto& day : days) {
        dayAssignments[day] = BodyPart::OTHER; // Default to OTHER/Rest
    }
}

std::string WorkoutRoutine::getName() const {
    return name;
}

void WorkoutRoutine::setName(const std::string& newName) {
    name = newName;
}

void WorkoutRoutine::assignDayToBodyPart(const std::string& day, BodyPart bodyPart) {
    // Convert to title case to ensure consistency
    std::string dayTitle = day;
    if (!dayTitle.empty()) {
        dayTitle[0] = std::toupper(dayTitle[0]);
        for (size_t i = 1; i < dayTitle.length(); ++i) {
            dayTitle[i] = std::tolower(dayTitle[i]);
        }
    }
    
    dayAssignments[dayTitle] = bodyPart;
}

BodyPart WorkoutRoutine::getBodyPartForDay(const std::string& day) const {
    // Convert to title case for lookup
    std::string dayTitle = day;
    if (!dayTitle.empty()) {
        dayTitle[0] = std::toupper(dayTitle[0]);
        for (size_t i = 1; i < dayTitle.length(); ++i) {
            dayTitle[i] = std::tolower(dayTitle[i]);
        }
    }
    
    auto it = dayAssignments.find(dayTitle);
    if (it != dayAssignments.end()) {
        return it->second;
    }
    return BodyPart::OTHER; // Default if day not found
}

bool WorkoutRoutine::isDayAssigned(const std::string& day) const {
    // Convert to title case for lookup
    std::string dayTitle = day;
    if (!dayTitle.empty()) {
        dayTitle[0] = std::toupper(dayTitle[0]);
        for (size_t i = 1; i < dayTitle.length(); ++i) {
            dayTitle[i] = std::tolower(dayTitle[i]);
        }
    }
    
    return dayAssignments.find(dayTitle) != dayAssignments.end();
}

void WorkoutRoutine::clearDayAssignment(const std::string& day) {
    // Convert to title case for lookup
    std::string dayTitle = day;
    if (!dayTitle.empty()) {
        dayTitle[0] = std::toupper(dayTitle[0]);
        for (size_t i = 1; i < dayTitle.length(); ++i) {
            dayTitle[i] = std::tolower(dayTitle[i]);
        }
    }
    
    dayAssignments.erase(dayTitle);
}

std::map<std::string, BodyPart> WorkoutRoutine::getDayAssignments() const {
    return dayAssignments;
}

std::string WorkoutRoutine::serialize() const {
    std::stringstream ss;
    ss << name << "|";
    
    // Save day assignments
    for (const auto& assignment : dayAssignments) {
        ss << assignment.first << "=" << static_cast<int>(assignment.second) << ";";
    }
    
    return ss.str();
}

WorkoutRoutine WorkoutRoutine::deserialize(const std::string& data) {
    std::string name;
    size_t nameEnd = data.find('|');
    if (nameEnd == std::string::npos) {
        return WorkoutRoutine("Default");
    }
    
    name = data.substr(0, nameEnd);
    WorkoutRoutine routine(name);
    
    // Parse day assignments
    std::string assignmentsStr = data.substr(nameEnd + 1);
    size_t pos = 0;
    
    while (pos < assignmentsStr.length()) {
        size_t equalPos = assignmentsStr.find('=', pos);
        if (equalPos == std::string::npos) break;
        
        size_t semicolonPos = assignmentsStr.find(';', equalPos);
        if (semicolonPos == std::string::npos) semicolonPos = assignmentsStr.length();
        
        std::string day = assignmentsStr.substr(pos, equalPos - pos);
        int bodyPartValue = std::stoi(assignmentsStr.substr(equalPos + 1, semicolonPos - equalPos - 1));
        BodyPart bodyPart = static_cast<BodyPart>(bodyPartValue);
        
        routine.assignDayToBodyPart(day, bodyPart);
        pos = semicolonPos + 1;
    }
    
    return routine;
}