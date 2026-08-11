#include "GymDatabse.h"
#include "Utils.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstdlib>

namespace {

std::string resolveDataFile(const std::string& filename) {
    if (filename != "gym_data.db" && filename != "gym_routines.db") {
        return filename;
    }

    const char* dataDirectory = std::getenv("GYMCLI_DATA_DIR");
    if (dataDirectory == nullptr || std::string(dataDirectory).empty()) {
        return filename;
    }

    std::string resolved(dataDirectory);
    if (resolved.back() != '/' && resolved.back() != '\\') {
        resolved += '/';
    }
    return resolved + filename;
}

bool isUnusedGeneratedDefault(const WorkoutRoutine& routine,
                              const std::vector<Exercise>& exercises) {
    if (routine.getName() != "Default Routine" ||
        !routine.getFocus().empty() || !routine.getNotes().empty() ||
        !routine.getExercises().empty()) {
        return false;
    }

    const std::map<std::string, BodyPart> expectedSchedule = {
        {"Monday", BodyPart::UPPER}, {"Tuesday", BodyPart::LOWER},
        {"Wednesday", BodyPart::FULL}, {"Thursday", BodyPart::UPPER},
        {"Friday", BodyPart::LOWER}, {"Saturday", BodyPart::FULL},
        {"Sunday", BodyPart::OTHER}
    };
    if (routine.getDayAssignments() != expectedSchedule) {
        return false;
    }

    for (const auto& exercise : exercises) {
        if (equalsIgnoreCase(exercise.getRoutineName(), routine.getName())) {
            return false;
        }
    }
    return true;
}

} // namespace

GymDatabase::GymDatabase(const std::string& filename, const std::string& routinesFile)
    : dbFilename(resolveDataFile(filename)),
      routinesFilename(resolveDataFile(routinesFile)),
      activeRoutineIndex(-1) {
    loadFromFile();
    loadRoutinesFromFile();

    // Older versions created a placeholder routine automatically. Remove only
    // that untouched, unused placeholder; preserve customized or used routines.
    for (size_t index = 0; index < routines.size(); ++index) {
        if (!isUnusedGeneratedDefault(routines[index], exercises)) {
            continue;
        }
        routines.erase(routines.begin() + index);
        if (routines.empty()) {
            activeRoutineIndex = -1;
        } else if (activeRoutineIndex == static_cast<int>(index)) {
            activeRoutineIndex = 0;
        } else if (activeRoutineIndex > static_cast<int>(index)) {
            --activeRoutineIndex;
        }
        saveRoutinesToFile();
        break;
    }

    // Recover cleanly from a stale or malformed active routine index.
    if (!routines.empty() &&
        (activeRoutineIndex < 0 || activeRoutineIndex >= static_cast<int>(routines.size()))) {
        activeRoutineIndex = 0;
        saveRoutinesToFile();
    }
}

void GymDatabase::addExercise(const Exercise& exercise) {
    exercises.push_back(exercise);
    saveToFile();
}

const std::vector<Exercise>& GymDatabase::getAllExercises() const {
    return exercises;
}

std::vector<Exercise> GymDatabase::getExercisesByName(const std::string& name) const {
    std::vector<Exercise> result;
    for (const auto& exercise : exercises) {
        if (containsIgnoreCase(exercise.getName(), name)) {
            result.push_back(exercise);
        }
    }
    return result;
}

std::vector<Exercise> GymDatabase::getExercisesByCategory(const std::string& category) const {
    std::vector<Exercise> result;
    for (const auto& exercise : exercises) {
        if (containsIgnoreCase(exercise.getCategory(), category)) {
            result.push_back(exercise);
        }
    }
    return result;
}

std::map<std::string, std::vector<Exercise>> GymDatabase::getSessionMap() const {
    std::map<std::string, std::vector<Exercise>> result;
    for (const auto& exercise : exercises) {
        result[exercise.getDate()].push_back(exercise);
    }
    return result;
}

std::vector<Exercise> GymDatabase::getExercisesByDay(const std::string& dayOfWeek) const {
    std::vector<Exercise> result;
    const std::string normalizedDay = normalizeDayOfWeek(dayOfWeek);
    if (normalizedDay.empty()) {
        return result;
    }

    for (const auto& exercise : exercises) {
        // Current records store YYYY-MM-DD. Older records stored a weekday;
        // keep those readable while using real dates going forward.
        if (equalsIgnoreCase(getDayForDateOrLegacy(exercise.getDate()), normalizedDay)) {
            result.push_back(exercise);
        }
    }
    return result;
}

std::vector<Exercise> GymDatabase::getExercisesByBodyPart(BodyPart bodyPart) const {
    std::vector<Exercise> result;
    for (const auto& exercise : exercises) {
        if (exercise.getBodyPart() == bodyPart) {
            result.push_back(exercise);
        }
    }
    return result;
}

std::vector<Exercise> GymDatabase::getExercisesByRoutine(const std::string& routineName) const {
    std::vector<Exercise> result;
    for (const auto& exercise : exercises) {
        if (equalsIgnoreCase(exercise.getRoutineName(), routineName)) {
            result.push_back(exercise);
        }
    }
    return result;
}

std::map<std::string, double> GymDatabase::getProgressData(const std::string& exerciseName) const {
    std::map<std::string, double> result;
    for (const auto& exercise : exercises) {
        if (equalsIgnoreCase(exercise.getName(), exerciseName)) {
            // Find the maximum weight used in this exercise
            double maxWeight = 0;
            for (const auto& weight : exercise.getWeights()) {
                maxWeight = std::max(maxWeight, weight);
            }
            result[exercise.getDate()] = std::max(result[exercise.getDate()], maxWeight);
        }
    }
    return result;
}

void GymDatabase::addRoutine(const WorkoutRoutine& routine) {
    routines.push_back(routine);

    // If this is the first routine, make it active
    if (routines.size() == 1) {
        activeRoutineIndex = 0;
    }

    saveRoutinesToFile();
}

bool GymDatabase::updateRoutine(size_t index, const WorkoutRoutine& routine) {
    if (index >= routines.size()) {
        return false;
    }

    const std::string previousName = routines[index].getName();
    routines[index] = routine;

    if (!equalsIgnoreCase(previousName, routine.getName())) {
        for (auto& exercise : exercises) {
            if (equalsIgnoreCase(exercise.getRoutineName(), previousName)) {
                exercise.setRoutineName(routine.getName());
            }
        }
        saveToFile();
    }

    saveRoutinesToFile();
    return true;
}

bool GymDatabase::deleteRoutine(size_t index) {
    if (index >= routines.size()) {
        return false;
    }

    routines.erase(routines.begin() + index);

    // Update active routine index if needed
    if (routines.empty()) {
        activeRoutineIndex = -1;
    } else if (activeRoutineIndex == static_cast<int>(index)) {
        activeRoutineIndex = 0;  // Default to first routine
    } else if (activeRoutineIndex > static_cast<int>(index)) {
        activeRoutineIndex--;  // Adjust index
    }

    saveRoutinesToFile();
    return true;
}

const std::vector<WorkoutRoutine>& GymDatabase::getAllRoutines() const {
    return routines;
}

const WorkoutRoutine* GymDatabase::getActiveRoutine() const {
    if (activeRoutineIndex >= 0 && activeRoutineIndex < static_cast<int>(routines.size())) {
        return &routines[activeRoutineIndex];
    }
    return nullptr;
}

bool GymDatabase::setActiveRoutine(size_t index) {
    if (index >= routines.size()) {
        return false;
    }

    activeRoutineIndex = static_cast<int>(index);
    saveRoutinesToFile();
    return true;
}

BodyPart GymDatabase::getBodyPartForToday() const {
    std::string todayDay = getCurrentDayOfWeek();
    return getBodyPartForDay(todayDay);
}

BodyPart GymDatabase::getBodyPartForDay(const std::string& day) const {
    if (activeRoutineIndex >= 0 && activeRoutineIndex < static_cast<int>(routines.size())) {
        return routines[activeRoutineIndex].getBodyPartForDay(day);
    }
    return BodyPart::OTHER;  // Default
}

void GymDatabase::saveToFile() const {
    std::ofstream file(dbFilename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file for writing: " << dbFilename << std::endl;
        return;
    }

    for (const auto& exercise : exercises) {
        file << exercise.serialize() << std::endl;
    }

    file.close();
}

void GymDatabase::loadFromFile() {
    exercises.clear();

    std::ifstream file(dbFilename);
    if (!file.is_open()) {
        // File doesn't exist yet, which is fine for a new database
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            exercises.push_back(Exercise::deserialize(line));
        }
    }

    file.close();
}

// Routine file I/O
void GymDatabase::saveRoutinesToFile() const {
    std::ofstream file(routinesFilename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open routines file for writing: " << routinesFilename << std::endl;
        return;
    }

    // Save active routine index
    file << activeRoutineIndex << std::endl;

    // Save routines
    for (const auto& routine : routines) {
        file << routine.serialize() << std::endl;
    }

    file.close();
}

void GymDatabase::loadRoutinesFromFile() {
    routines.clear();
    activeRoutineIndex = -1;

    std::ifstream file(routinesFilename);
    if (!file.is_open()) {
        // File doesn't exist yet, which is fine for a new database
        return;
    }

    std::string line;

    // Read active routine index
    if (std::getline(file, line)) {
        try {
            activeRoutineIndex = std::stoi(line);
        } catch (...) {
            activeRoutineIndex = -1;
        }
    }

    // Read routines
    while (std::getline(file, line)) {
        if (!line.empty()) {
            routines.push_back(WorkoutRoutine::deserialize(line));
        }
    }

    file.close();
}
