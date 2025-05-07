#include "TableRenderer.h"
#include "Utils.h"  // For utility functions
#include <iostream>
#include <iomanip>
#include <algorithm>

void TableRenderer::renderExercise(const Exercise& exercise) {
    int nameWidth = std::max(20, static_cast<int>(exercise.getName().length() + 4));
    int totalWidth = nameWidth + 40;

    // Get day of week for the date
    std::string dayOfWeek = getDayOfWeek(exercise.getDate());

    std::string title = " " + exercise.getName() + " (" + exercise.getCategory() + ") - " +
                        exercise.getDate() + " (" + dayOfWeek + ") ";
    int padding = (totalWidth - title.length()) / 2;

    // Print header
    std::cout << std::string(padding, '=') << title << std::string(padding + title.length() % 2, '=') << std::endl;

    // Get body part string and routine info
    std::string bodyPartStr = exercise.getBodyPartString();
    std::string routineName = exercise.getRoutineName();

    std::cout << " Body Part: " << bodyPartStr << std::endl;
    if (!routineName.empty()) {
        std::cout << " Routine: " << routineName << std::endl;
    }

    // Print sets table
    if (exercise.getMeasurementType() == MeasurementType::REPS) {
        std::cout << std::setw(8) << "Set" << std::setw(8) << "Reps" << std::setw(10) << "Weight(kg)" << std::setw(14) << "Volume" << std::endl;
    } else {
        std::cout << std::setw(8) << "Set" << std::setw(8) << "Time" << std::setw(10) << "Weight(kg)" << std::setw(14) << "Volume" << std::endl;
    }

    std::cout << std::string(totalWidth, '-') << std::endl;

    const auto& sets = exercise.getSets();
    const auto& reps = exercise.getReps();
    const auto& duration = exercise.getDuration();
    const auto& weights = exercise.getWeights();

    for (size_t i = 0; i < sets.size(); i++) {
        double setVolume;

        if (exercise.getMeasurementType() == MeasurementType::REPS) {
            setVolume = reps[i] * weights[i];
            std::cout << std::setw(8) << sets[i]
                      << std::setw(8) << reps[i]
                      << std::setw(10) << std::fixed << std::setprecision(1) << weights[i]
                      << std::setw(14) << std::fixed << std::setprecision(1) << setVolume
                      << std::endl;
        } else {
            setVolume = (duration[i] / 60.0) * weights[i]; // Convert seconds to minutes
            std::cout << std::setw(8) << sets[i]
                      << std::setw(8) << Exercise::formatTime(duration[i])
                      << std::setw(10) << std::fixed << std::setprecision(1) << weights[i]
                      << std::setw(14) << std::fixed << std::setprecision(1) << setVolume
                      << std::endl;
        }
    }

    // Print total volume
    std::cout << std::string(totalWidth, '-') << std::endl;
    std::cout << std::setw(26) << "Total Volume: "
              << std::setw(14) << std::fixed << std::setprecision(1) << exercise.calculateVolume()
              << std::endl;

    // Print notes if any
    if (!exercise.getNotes().empty()) {
        std::cout << std::endl << "Notes: " << exercise.getNotes() << std::endl;
    }

    std::cout << std::string(totalWidth, '=') << std::endl << std::endl;
}

void TableRenderer::renderExerciseList(const std::vector<Exercise>& exercises) {
    if (exercises.empty()) {
        std::cout << "No exercises found." << std::endl;
        return;
    }

    int totalWidth = 106; // Increased width for routine column
    std::cout << std::string(totalWidth, '=') << std::endl;
    std::cout << std::setw(6) << "ID"
              << std::setw(20) << "Exercise Name"
              << std::setw(15) << "Category"
              << std::setw(15) << "Date"
              << std::setw(12) << "Day"
              << std::setw(15) << "Body Part"
              << std::setw(15) << "Routine" // Added routine column
              << std::setw(6) << "Sets"
              << std::setw(6) << "Volume" << std::endl;
    std::cout << std::string(totalWidth, '-') << std::endl;

    for (size_t i = 0; i < exercises.size(); i++) {
        // Get day of week for the date
        std::string dayOfWeek = getDayOfWeek(exercises[i].getDate());

        std::cout << std::setw(6) << (i + 1)
                  << std::setw(20) << exercises[i].getName().substr(0, 18)
                  << std::setw(15) << exercises[i].getCategory().substr(0, 13)
                  << std::setw(15) << exercises[i].getDate()
                  << std::setw(12) << dayOfWeek.substr(0, 10)
                  << std::setw(15) << exercises[i].getBodyPartString()
                  << std::setw(15) << exercises[i].getRoutineName().substr(0, 13) // Display routine name
                  << std::setw(7) << exercises[i].getSets().size()
                  << std::setw(7) << std::fixed << std::setprecision(1) << exercises[i].calculateVolume()
                  << std::endl;
    }

    std::cout << std::string(totalWidth, '=') << std::endl;
}

void TableRenderer::renderSessionList(const std::map<std::string, std::vector<Exercise>>& sessionMap) {
    if (sessionMap.empty()) {
        std::cout << "No workout sessions found." << std::endl;
        return;
    }

    int totalWidth = 120; // Increased for routine information
    std::cout << std::string(totalWidth, '=') << std::endl;
    std::cout << std::setw(15) << "Date"
              << std::setw(15) << "Day of Week"
              << std::setw(10) << "Exercises"
              << std::setw(15) << "Categories"
              << std::setw(30) << "Body Parts"
              << std::setw(15) << "Routines" // Added routines column
              << std::setw(10) << "Sets"
              << std::setw(7) << "Volume" << std::endl;
    std::cout << std::string(totalWidth, '-') << std::endl;

    // For each date
    for (const auto& session : sessionMap) {
        // Get the day of week for this session using the utility function
        std::string dayOfWeek = getDayOfWeek(session.first);

        // Count unique categories, body parts, and routines
        std::map<std::string, bool> categories;
        std::map<std::string, bool> bodyParts;
        std::map<std::string, bool> routines;
        int totalSets = 0;
        double totalVolume = 0;

        for (const auto& exercise : session.second) {
            categories[exercise.getCategory()] = true;
            bodyParts[exercise.getBodyPartString()] = true;
            if (!exercise.getRoutineName().empty()) {
                routines[exercise.getRoutineName()] = true;
            }

            totalSets += exercise.getSets().size();
            totalVolume += exercise.calculateVolume();
        }

        // Create a string of body parts
        std::string bodyPartList;
        for (const auto& bp : bodyParts) {
            if (!bodyPartList.empty()) {
                bodyPartList += ", ";
            }
            bodyPartList += bp.first;
        }

        // Create a string of routines
        std::string routineList;
        for (const auto& r : routines) {
            if (!routineList.empty()) {
                routineList += ", ";
            }
            routineList += r.first;
        }

        // Format the categories count properly
        std::string categoriesText = std::to_string(categories.size());

        std::cout << std::setw(15) << session.first
                  << std::setw(15) << dayOfWeek
                  << std::setw(10) << session.second.size()
                  << std::setw(15) << categoriesText
                  << std::setw(30) << bodyPartList
                  << std::setw(15) << routineList.substr(0, 13)
                  << std::setw(10) << totalSets
                  << std::setw(10) << std::fixed << std::setprecision(1) << totalVolume
                  << std::endl;
    }

    std::cout << std::string(totalWidth, '=') << std::endl;
}

void TableRenderer::renderProgressChart(const std::string& exerciseName,
                               const std::map<std::string, double>& progressData) {
    if (progressData.empty()) {
        std::cout << "No progress data available for " << exerciseName << std::endl;
        return;
    }

    int totalWidth = 80;
    std::cout << std::string(totalWidth, '=') << std::endl;
    std::cout << " Progress Chart for " << exerciseName << " " << std::endl;
    std::cout << std::string(totalWidth, '-') << std::endl;

    // Find max value for scaling
    double maxValue = 0;
    for (const auto& entry : progressData) {
        maxValue = std::max(maxValue, entry.second);
    }

    const int chartWidth = 50;

    // For each date
    for (const auto& entry : progressData) {
        int barLength = static_cast<int>((entry.second / maxValue) * chartWidth);
        std::cout << std::setw(12) << entry.first << " |";
        std::cout << std::string(barLength, '#');
        std::cout << " " << std::fixed << std::setprecision(1) << entry.second << std::endl;
    }

    std::cout << std::string(totalWidth, '=') << std::endl;
}
