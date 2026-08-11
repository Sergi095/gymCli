#include "WorkoutRoutine.h"
#include "Utils.h"
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <cctype>

namespace {

std::string encodeField(const std::string& value) {
    std::ostringstream encoded;
    encoded << std::uppercase << std::hex;
    for (unsigned char character : value) {
        if (std::isalnum(character) || character == '-' || character == '_' ||
            character == '.' || character == '~') {
            encoded << static_cast<char>(character);
        } else {
            encoded << '%' << std::setw(2) << std::setfill('0')
                    << static_cast<int>(character);
        }
    }
    return encoded.str();
}

int hexValue(char character) {
    if (character >= '0' && character <= '9') return character - '0';
    if (character >= 'a' && character <= 'f') return character - 'a' + 10;
    if (character >= 'A' && character <= 'F') return character - 'A' + 10;
    return -1;
}

std::string decodeField(const std::string& value) {
    std::string decoded;
    for (size_t index = 0; index < value.size(); ++index) {
        if (value[index] == '%' && index + 2 < value.size()) {
            const int high = hexValue(value[index + 1]);
            const int low = hexValue(value[index + 2]);
            if (high >= 0 && low >= 0) {
                decoded += static_cast<char>((high << 4) | low);
                index += 2;
                continue;
            }
        }
        decoded += value[index];
    }
    return decoded;
}

std::vector<std::string> split(const std::string& value, char delimiter) {
    std::vector<std::string> parts;
    size_t start = 0;
    while (start <= value.size()) {
        const size_t end = value.find(delimiter, start);
        if (end == std::string::npos) {
            parts.push_back(value.substr(start));
            break;
        }
        parts.push_back(value.substr(start, end - start));
        start = end + 1;
    }
    return parts;
}

} // namespace

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

std::string WorkoutRoutine::getNotes() const {
    return notes;
}

void WorkoutRoutine::setNotes(const std::string& newNotes) {
    notes = newNotes;
}

std::string WorkoutRoutine::getFocus() const {
    return focus;
}

void WorkoutRoutine::setFocus(const std::string& newFocus) {
    focus = newFocus;
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

void WorkoutRoutine::addExercise(const RoutineExercise& exercise) {
    exercises.push_back(exercise);
}

const std::vector<RoutineExercise>& WorkoutRoutine::getExercises() const {
    return exercises;
}

std::string WorkoutRoutine::serialize() const {
    std::stringstream ss;
    ss << name << "|";
    
    // Save day assignments
    for (const auto& assignment : dayAssignments) {
        ss << assignment.first << "=" << static_cast<int>(assignment.second) << ";";
    }
    ss << "|v2|" << encodeField(notes) << "|" << encodeField(focus) << "|";

    for (size_t index = 0; index < exercises.size(); ++index) {
        const RoutineExercise& exercise = exercises[index];
        if (index > 0) {
            ss << "~";
        }
        ss << encodeField(exercise.section) << "^"
           << encodeField(exercise.name) << "^"
           << encodeField(exercise.sets) << "^"
           << encodeField(exercise.reps) << "^"
           << encodeField(exercise.duration) << "^"
           << encodeField(exercise.weight) << "^"
           << encodeField(exercise.rest) << "^"
           << encodeField(exercise.notes) << "^"
           << encodeField(exercise.link);
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
    
    // Older records end after assignments. Version 2 records append notes and exercises.
    const size_t assignmentsEnd = data.find('|', nameEnd + 1);
    std::string assignmentsStr = assignmentsEnd == std::string::npos
        ? data.substr(nameEnd + 1)
        : data.substr(nameEnd + 1, assignmentsEnd - nameEnd - 1);
    size_t pos = 0;
    
    while (pos < assignmentsStr.length()) {
        size_t equalPos = assignmentsStr.find('=', pos);
        if (equalPos == std::string::npos) break;
        
        size_t semicolonPos = assignmentsStr.find(';', equalPos);
        if (semicolonPos == std::string::npos) semicolonPos = assignmentsStr.length();
        
        std::string day = assignmentsStr.substr(pos, equalPos - pos);
        try {
            int bodyPartValue = std::stoi(
                assignmentsStr.substr(equalPos + 1, semicolonPos - equalPos - 1));
            if (bodyPartValue < static_cast<int>(BodyPart::UPPER) ||
                bodyPartValue > static_cast<int>(BodyPart::OTHER)) {
                bodyPartValue = static_cast<int>(BodyPart::OTHER);
            }
            routine.assignDayToBodyPart(day, static_cast<BodyPart>(bodyPartValue));
        } catch (...) {
            // Leave the default rest-day assignment for malformed entries.
        }
        pos = semicolonPos + 1;
    }

    if (assignmentsEnd == std::string::npos) {
        return routine;
    }

    const std::string versionedData = data.substr(assignmentsEnd + 1);
    if (versionedData.compare(0, 3, "v2|") != 0) {
        return routine;
    }

    const std::string payload = versionedData.substr(3);
    const size_t notesEnd = payload.find('|');
    if (notesEnd == std::string::npos) {
        return routine;
    }
    routine.setNotes(decodeField(payload.substr(0, notesEnd)));

    const size_t focusEnd = payload.find('|', notesEnd + 1);
    if (focusEnd == std::string::npos) {
        return routine;
    }
    routine.setFocus(decodeField(payload.substr(notesEnd + 1, focusEnd - notesEnd - 1)));

    const std::string exerciseData = payload.substr(focusEnd + 1);
    if (exerciseData.empty()) {
        return routine;
    }
    for (const auto& record : split(exerciseData, '~')) {
        const std::vector<std::string> fields = split(record, '^');
        if (fields.size() != 9) {
            continue;
        }
        RoutineExercise exercise;
        exercise.section = decodeField(fields[0]);
        exercise.name = decodeField(fields[1]);
        exercise.sets = decodeField(fields[2]);
        exercise.reps = decodeField(fields[3]);
        exercise.duration = decodeField(fields[4]);
        exercise.weight = decodeField(fields[5]);
        exercise.rest = decodeField(fields[6]);
        exercise.notes = decodeField(fields[7]);
        exercise.link = decodeField(fields[8]);
        routine.addExercise(exercise);
    }

    return routine;
}
