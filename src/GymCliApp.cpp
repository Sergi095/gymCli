#include "GymCliApp.h"
#include "TableRenderer.h"
#include <iostream>    // For cout, cin, endl
#include <iomanip>     // For setw, left, put_time
#include <algorithm>   // For std::transform
#include <limits>      // For numeric_limits
#include <cctype>      // For ::tolower, toupper
#include <map>         // For std::map
#include <sstream>     // For std::ostringstream
#include <chrono>      // For std::chrono
#include <ctime>       // For std::time_t, std::localtime
#include <vector>      // For std::vector

GymCliApp::GymCliApp() : running(true) {}

void GymCliApp::displayMenu() {
    int width = 45;
    std::cout << std::string(width, '*') << std::endl;
    std::cout << "*" << std::setw(width - 2) << std::left << " GYM CLI - MAIN MENU" << "*" << std::endl;
    std::cout << "*" << std::setw(width - 2) << std::left << " Today is " + getCurrentDayOfWeek() << "*" << std::endl;

    // Display today's workout based on active routine
    BodyPart todayBodyPart = db.getBodyPartForToday();
    std::string bodyPartStr;

    switch(todayBodyPart) {
        case BodyPart::UPPER: bodyPartStr = "Upper Body"; break;
        case BodyPart::LOWER: bodyPartStr = "Lower Body"; break;
        case BodyPart::FULL: bodyPartStr = "Full Body"; break;
        default: bodyPartStr = "Rest Day"; break;
    }

    std::cout << "*" << std::setw(width - 2) << std::left << " Today's Workout: " + bodyPartStr << "*" << std::endl;

    // Display active routine name
    const WorkoutRoutine* activeRoutine = db.getActiveRoutine();
    if (activeRoutine) {
        std::cout << "*" << std::setw(width - 2) << std::left << " Active Routine: " + activeRoutine->getName() << "*" << std::endl;
    }

    std::cout << std::string(width, '*') << std::endl;
    std::cout << "1. Add new exercise" << std::endl;
    std::cout << "2. View all exercises" << std::endl;
    std::cout << "3. View exercises by category" << std::endl;
    std::cout << "4. View exercises by name" << std::endl;
    std::cout << "5. View exercises by day of week" << std::endl;
    std::cout << "6. View exercises by body part (upper/lower)" << std::endl;
    std::cout << "7. View exercises by routine" << std::endl;
    std::cout << "8. View workout sessions by date" << std::endl;
    std::cout << "9. View progress for an exercise" << std::endl;
    std::cout << "10. Manage workout routines" << std::endl;
    std::cout << "0. Exit" << std::endl;
    std::cout << std::string(width, '*') << std::endl;
    std::cout << "Enter your choice: ";
}

// Helper functions for string processing and categorization
bool GymCliApp::containsIgnoreCase(const std::string& str, const std::string& substr) {
    std::string strLower = str;
    std::string substrLower = substr;
    
    // Convert both strings to lowercase
    std::transform(strLower.begin(), strLower.end(), strLower.begin(), 
                   [](unsigned char c) { return std::tolower(c); });
    std::transform(substrLower.begin(), substrLower.end(), substrLower.begin(), 
                   [](unsigned char c) { return std::tolower(c); });
    
    // Check if strLower contains substrLower
    return strLower.find(substrLower) != std::string::npos;
}

bool GymCliApp::isUpperBodyCategory(const std::string& category) {
    return containsIgnoreCase(category, "chest") ||
           containsIgnoreCase(category, "back") ||
           containsIgnoreCase(category, "shoulder") ||
           containsIgnoreCase(category, "arm") ||
           containsIgnoreCase(category, "bicep") ||
           containsIgnoreCase(category, "tricep") ||
           containsIgnoreCase(category, "push") ||
           containsIgnoreCase(category, "pull") ||
           containsIgnoreCase(category, "press") ||
           containsIgnoreCase(category, "curl") ||
           containsIgnoreCase(category, "delt") ||
           containsIgnoreCase(category, "trap") ||
           containsIgnoreCase(category, "lats") ||
           containsIgnoreCase(category, "upper");
}

bool GymCliApp::isLowerBodyCategory(const std::string& category) {
    return containsIgnoreCase(category, "leg") ||
           containsIgnoreCase(category, "quad") ||
           containsIgnoreCase(category, "hamstring") ||
           containsIgnoreCase(category, "calf") ||
           containsIgnoreCase(category, "calves") ||
           containsIgnoreCase(category, "glute") ||
           containsIgnoreCase(category, "squat") ||
           containsIgnoreCase(category, "deadlift") ||
           containsIgnoreCase(category, "lunge") ||
           containsIgnoreCase(category, "hip") ||
           containsIgnoreCase(category, "thigh") ||
           containsIgnoreCase(category, "knee") ||
           containsIgnoreCase(category, "lower");
}

BodyPart GymCliApp::getCategoryBodyPart(const std::string& category) {
    if (isUpperBodyCategory(category)) {
        return BodyPart::UPPER;
    } else if (isLowerBodyCategory(category)) {
        return BodyPart::LOWER;
    } else if (containsIgnoreCase(category, "full") ||
              containsIgnoreCase(category, "cardio") ||
              containsIgnoreCase(category, "core") ||
              containsIgnoreCase(category, "abs") ||
              containsIgnoreCase(category, "hiit") ||
              containsIgnoreCase(category, "circuit") ||
              containsIgnoreCase(category, "functional")) {
        return BodyPart::FULL;
    } else {
        return BodyPart::OTHER;
    }
}

// Helper functions for time and date
std::vector<std::string> GymCliApp::getDaysOfWeek() {
    return {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
}

std::string GymCliApp::getCurrentDayOfWeek() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm today_tm = *std::localtime(&now_time);
    
    std::vector<std::string> days = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    int dayIndex = today_tm.tm_wday; // 0 = Sunday
    
    return days[dayIndex];
}

std::string GymCliApp::getCurrentDate() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm today_tm = *std::localtime(&now_time);
    
    std::ostringstream oss;
    oss << std::put_time(&today_tm, "%Y-%m-%d");
    return oss.str();
}

std::string GymCliApp::getDateForDayOfWeek(const std::string& dayOfWeek) {
    // Just return the day of week instead of converting to a date
    return dayOfWeek;
}

// Main exercise management functions
void GymCliApp::addNewExercise() {
    std::string name, category, notes;

    std::cout << "Enter exercise name: ";
    std::cin.ignore();
    std::getline(std::cin, name);

    // Get category first, to determine body part
    std::cout << "Enter category (e.g., Chest, Back, Legs): ";
    std::getline(std::cin, category);

    // Determine body part based on category
    BodyPart exerciseBodyPart = getCategoryBodyPart(category);
    
    // Display detected body part
    std::string bodyPartStr;
    switch (exerciseBodyPart) {
        case BodyPart::UPPER: bodyPartStr = "Upper Body (U)"; break;
        case BodyPart::LOWER: bodyPartStr = "Lower Body (L)"; break;
        case BodyPart::FULL: bodyPartStr = "Full Body (F)"; break;
        default: bodyPartStr = "Other (O)"; break;
    }
    
    std::cout << "Detected body part: " << bodyPartStr << std::endl;
    
    // Confirm or change body part
    char bodyPartChoice;
    std::cout << "Is this correct? [Y/n]: ";
    std::string choiceStr;
    std::getline(std::cin, choiceStr);
    bodyPartChoice = choiceStr.empty() ? 'Y' : toupper(choiceStr[0]);
    
    if (bodyPartChoice != 'Y') {
        std::cout << "Select body part [U/L/F/O]: ";
        std::getline(std::cin, choiceStr);
        char selection = choiceStr.empty() ? 'U' : toupper(choiceStr[0]);
        
        switch (selection) {
            case 'U': exerciseBodyPart = BodyPart::UPPER; break;
            case 'L': exerciseBodyPart = BodyPart::LOWER; break;
            case 'F': exerciseBodyPart = BodyPart::FULL; break;
            default: exerciseBodyPart = BodyPart::OTHER; break;
        }
        
        switch (exerciseBodyPart) {
            case BodyPart::UPPER: bodyPartStr = "Upper Body"; break;
            case BodyPart::LOWER: bodyPartStr = "Lower Body"; break;
            case BodyPart::FULL: bodyPartStr = "Full Body"; break;
            default: bodyPartStr = "Other"; break;
        }
        
        std::cout << "Body part set to: " << bodyPartStr << std::endl;
    }
    
    // Check if we have an active routine
    const WorkoutRoutine* activeRoutine = db.getActiveRoutine();
    std::string chosenDay;
    
    if (activeRoutine) {
        // Find days that match this body part in the routine
        std::vector<std::string> matchingDays;
        for (const std::string& day : getDaysOfWeek()) {
            if (activeRoutine->getBodyPartForDay(day) == exerciseBodyPart) {
                matchingDays.push_back(day);
            }
        }
        
        if (!matchingDays.empty()) {
            std::cout << "Based on your active routine \"" << activeRoutine->getName() 
                      << "\", this exercise fits on:" << std::endl;
            
            for (size_t i = 0; i < matchingDays.size(); i++) {
                std::cout << (i+1) << ". " << matchingDays[i] << std::endl;
            }
            
            int dayChoice = 0;
            if (matchingDays.size() > 1) {
                std::cout << "Select day (1-" << matchingDays.size() << "): ";
                std::string choiceStr;
                std::getline(std::cin, choiceStr);
                try {
                    dayChoice = std::stoi(choiceStr);
                    if (dayChoice < 1 || dayChoice > static_cast<int>(matchingDays.size())) {
                        dayChoice = 1; // Default to first matching day
                    }
                } catch (...) {
                    dayChoice = 1; // Default to first matching day
                }
            } else {
                dayChoice = 1; // Only one matching day
            }
            
            chosenDay = matchingDays[dayChoice - 1];
        } else {
            std::cout << "No matching days found in your active routine for this body part." << std::endl;
            std::cout << "Available days in your routine:" << std::endl;
            
            // Display all days in the routine with their body parts
            for (const std::string& day : getDaysOfWeek()) {
                BodyPart bp = activeRoutine->getBodyPartForDay(day);
                std::string bpStr;
                switch (bp) {
                    case BodyPart::UPPER: bpStr = "Upper Body"; break;
                    case BodyPart::LOWER: bpStr = "Lower Body"; break;
                    case BodyPart::FULL: bpStr = "Full Body"; break;
                    default: bpStr = "Rest Day"; break;
                }
                std::cout << "  - " << day << ": " << bpStr << std::endl;
            }
            
            // Let user choose a day manually
            std::cout << "Enter day to assign (e.g., Monday): ";
            std::getline(std::cin, chosenDay);
            std::transform(chosenDay.begin(), chosenDay.end(), chosenDay.begin(), ::tolower);
            chosenDay[0] = toupper(chosenDay[0]); // Capitalize first letter
        }
    } else {
        // No active routine, just use today's date
        chosenDay = getCurrentDayOfWeek();
        std::cout << "No active routine found. Using today (" << chosenDay << ")." << std::endl;
    }
    
    std::cout << "Chosen day: " << chosenDay << std::endl;

    std::string routineName = "";
    if (activeRoutine) {
        char associateChoice;
        std::cout << "Associate with active routine \"" << activeRoutine->getName() << "\"? [Y/n]: ";
        std::string choiceStr;
        std::getline(std::cin, choiceStr);
        associateChoice = choiceStr.empty() ? 'Y' : choiceStr[0];

        if (toupper(associateChoice) != 'N') {
            routineName = activeRoutine->getName();
        }
    }

    char typeChoice;
    std::cout << "Is this a rep-based exercise (R) or time-based exercise (T)? [R/T]: ";
    std::string typeChoiceStr;
    std::getline(std::cin, typeChoiceStr);
    typeChoice = typeChoiceStr.empty() ? 'R' : typeChoiceStr[0];

    MeasurementType measureType = (toupper(typeChoice) == 'T') ?
                                  MeasurementType::TIME :
                                  MeasurementType::REPS;

    // Create exercise with the day of week instead of a date
    Exercise exercise(name, category, measureType, exerciseBodyPart);
    exercise.setDate(chosenDay);  // Just using the day of week, not actual date
    exercise.setRoutineName(routineName);

    int numSets;
    std::cout << "Enter number of sets: ";
    std::string numSetsStr;
    std::getline(std::cin, numSetsStr);
    try {
        numSets = std::stoi(numSetsStr);
    } catch (...) {
        std::cout << "Invalid number of sets. Setting to 1." << std::endl;
        numSets = 1;
    }

    if (numSets <= 0 || numSets > 100) {
        std::cout << "Invalid number of sets (must be between 1 and 100). Setting to 1." << std::endl;
        numSets = 1;
    }

    for (int i = 0; i < numSets; i++) {
        double weight;

        if (measureType == MeasurementType::REPS) {
            int reps;
            std::cout << "Set " << (i + 1) << " - Enter reps: ";
            std::string repsStr;
            std::getline(std::cin, repsStr);
            try {
                reps = std::stoi(repsStr);
            } catch (...) {
                reps = 1;
            }

            if (reps <= 0 || reps > 1000) reps = 1;

            std::cout << "Set " << (i + 1) << " - Enter weight (kg): ";
            std::string weightStr;
            std::getline(std::cin, weightStr);
            try {
                weight = std::stod(weightStr);
            } catch (...) {
                weight = 0.0;
            }

            if (weight < 0 || weight > 1000) weight = 0.0;

            exercise.addRepSet(reps, weight);
        } else {
            char timeFormat;
            std::cout << "Set " << (i + 1) << " - Select time format [S/M]: ";
            std::string formatStr;
            std::getline(std::cin, formatStr);
            timeFormat = formatStr.empty() ? 'S' : formatStr[0];

            int totalSeconds = 0;

            if (toupper(timeFormat) == 'M') {
                int minutes = 0, seconds = 0;

                std::cout << "Set " << (i + 1) << " - Minutes: ";
                std::getline(std::cin, formatStr);
                try { minutes = std::stoi(formatStr); } catch (...) {}

                std::cout << "Set " << (i + 1) << " - Seconds: ";
                std::getline(std::cin, formatStr);
                try { seconds = std::stoi(formatStr); } catch (...) {}

                totalSeconds = minutes * 60 + seconds;
            } else {
                std::cout << "Set " << (i + 1) << " - Enter total seconds: ";
                std::getline(std::cin, formatStr);
                try { totalSeconds = std::stoi(formatStr); } catch (...) {}
            }

            if (totalSeconds <= 0 || totalSeconds > 3600) totalSeconds = 1;

            std::cout << "Set " << (i + 1) << " - Enter weight (kg): ";
            std::string weightStr;
            std::getline(std::cin, weightStr);
            try {
                weight = std::stod(weightStr);
            } catch (...) {
                weight = 0.0;
            }

            if (weight < 0 || weight > 1000) weight = 0.0;

            exercise.addTimeSet(totalSeconds, weight);
        }
    }

    std::cout << "Enter notes (optional): ";
    std::getline(std::cin, notes);
    exercise.setNotes(notes);

    db.addExercise(exercise);
    std::cout << "Exercise added successfully!" << std::endl;

    TableRenderer::renderExercise(exercise);
}

void GymCliApp::viewAllExercises() {
    const auto& exercises = db.getAllExercises();
    TableRenderer::renderExerciseList(exercises);

    if (!exercises.empty()) {
        std::cout << "Enter exercise ID to view details (0 to return): ";
        int id;
        std::cin >> id;

        if (id > 0 && id <= static_cast<int>(exercises.size())) {
            TableRenderer::renderExercise(exercises[id - 1]);
        }
    }
}

void GymCliApp::viewExercisesByCategory() {
    std::string category;
    std::cout << "Enter category to view: ";
    std::cin.ignore();
    std::getline(std::cin, category);

    const auto& exercises = db.getExercisesByCategory(category);
    TableRenderer::renderExerciseList(exercises);

    if (!exercises.empty()) {
        std::cout << "Enter exercise ID to view details (0 to return): ";
        int id;
        std::cin >> id;

        if (id > 0 && id <= static_cast<int>(exercises.size())) {
            TableRenderer::renderExercise(exercises[id - 1]);
        }
    }
}

void GymCliApp::viewExercisesByName() {
    std::string name;
    std::cout << "Enter exercise name to view: ";
    std::cin.ignore();
    std::getline(std::cin, name);

    const auto& exercises = db.getExercisesByName(name);
    TableRenderer::renderExerciseList(exercises);

    if (!exercises.empty()) {
        std::cout << "Enter exercise ID to view details (0 to return): ";
        int id;
        std::cin >> id;

        if (id > 0 && id <= static_cast<int>(exercises.size())) {
            TableRenderer::renderExercise(exercises[id - 1]);
        }
    }
}

void GymCliApp::viewExercisesByDay() {
    std::string dayOfWeek;
    std::cout << "Enter day of week (e.g., Monday, Tuesday): ";
    std::cin.ignore();
    std::getline(std::cin, dayOfWeek);
    
    // Normalize day input (lowercase with first letter capitalized)
    if (!dayOfWeek.empty()) {
        // Convert to lowercase first
        std::transform(dayOfWeek.begin(), dayOfWeek.end(), dayOfWeek.begin(), 
                      [](unsigned char c) { return std::tolower(c); });
        
        // Capitalize first letter
        dayOfWeek[0] = std::toupper(dayOfWeek[0]);
    }
    
    // Debug output
    std::cout << "Searching for exercises on: " << dayOfWeek << std::endl;
    
    const auto& exercises = db.getExercisesByDay(dayOfWeek);
    
    if (exercises.empty()) {
        std::cout << "No exercises found for " << dayOfWeek << "." << std::endl;
    } else {
        std::cout << "Found " << exercises.size() << " exercise(s) for " << dayOfWeek << ":" << std::endl;
        TableRenderer::renderExerciseList(exercises);

        std::cout << "Enter exercise ID to view details (0 to return): ";
        int id;
        std::cin >> id;

        if (id > 0 && id <= static_cast<int>(exercises.size())) {
            TableRenderer::renderExercise(exercises[id - 1]);
        }
    }
}

void GymCliApp::viewExercisesByBodyPart() {
    char choice;
    std::cout << "Select body part:\n"
              << "U - Upper body\n"
              << "L - Lower body\n"
              << "F - Full body\n"
              << "O - Other\n"
              << "Choice: ";
    std::cin >> choice;

    BodyPart bodyPart;
    switch (toupper(choice)) {
        case 'U': bodyPart = BodyPart::UPPER; break;
        case 'L': bodyPart = BodyPart::LOWER; break;
        case 'F': bodyPart = BodyPart::FULL; break;
        default:  bodyPart = BodyPart::OTHER; break;
    }

    const auto& exercises = db.getExercisesByBodyPart(bodyPart);
    TableRenderer::renderExerciseList(exercises);

    if (!exercises.empty()) {
        std::cout << "Enter exercise ID to view details (0 to return): ";
        int id;
        std::cin >> id;

        if (id > 0 && id <= static_cast<int>(exercises.size())) {
            TableRenderer::renderExercise(exercises[id - 1]);
        }
    }
}

void GymCliApp::viewExercisesByRoutine() {
    const auto& routines = db.getAllRoutines();
    if (routines.empty()) {
        std::cout << "No routines found." << std::endl;
        return;
    }

    // Display available routines
    std::cout << "Available routines:" << std::endl;
    for (size_t i = 0; i < routines.size(); i++) {
        std::cout << (i + 1) << ". " << routines[i].getName() <<
                     (db.getActiveRoutine() == &routines[i] ? " (Active)" : "") << std::endl;
    }

    size_t routineId;
    std::cout << "Enter routine ID to view exercises (0 to return): ";
    std::cin >> routineId;

    if (routineId == 0 || routineId > routines.size()) {
        return;
    }

    // Get exercises for the selected routine
    const auto& exercises = db.getExercisesByRoutine(routines[routineId - 1].getName());

    if (exercises.empty()) {
        std::cout << "No exercises found for this routine." << std::endl;
        return;
    }

    TableRenderer::renderExerciseList(exercises);

    std::cout << "Enter exercise ID to view details (0 to return): ";
    int id;
    std::cin >> id;

    if (id > 0 && id <= static_cast<int>(exercises.size())) {
        TableRenderer::renderExercise(exercises[id - 1]);
    }
}

void GymCliApp::viewWorkoutSessions() {
    const auto& sessionMap = db.getSessionMap();
    TableRenderer::renderSessionList(sessionMap);

    if (!sessionMap.empty()) {
        std::string date;
        std::cout << "Enter date to view session details (YYYY-MM-DD, or 0 to return): ";
        std::cin.ignore();
        std::getline(std::cin, date);

        if (date != "0" && sessionMap.find(date) != sessionMap.end()) {
            const auto& exercises = sessionMap.at(date);
            TableRenderer::renderExerciseList(exercises);

            std::cout << "Enter exercise ID to view details (0 to return): ";
            int id;
            std::cin >> id;

            if (id > 0 && id <= static_cast<int>(exercises.size())) {
                TableRenderer::renderExercise(exercises[id - 1]);
            }
        }
    }
}

void GymCliApp::viewProgressForExercise() {
    std::string name;
    std::cout << "Enter exercise name to view progress: ";
    std::cin.ignore();
    std::getline(std::cin, name);

    const auto& progressData = db.getProgressData(name);
    TableRenderer::renderProgressChart(name, progressData);
}

// Routine management functions
void GymCliApp::manageRoutines() {
    bool backToMainMenu = false;

    while (!backToMainMenu) {
        displayRoutinesMenu();

        int choice;
        std::cin >> choice;

        switch (choice) {
            case 0:
                backToMainMenu = true;
                break;
            case 1:
                viewAllRoutines();
                break;
            case 2:
                addNewRoutine();
                break;
            case 3:
                editRoutine();
                break;
            case 4:
                deleteRoutine();
                break;
            case 5:
                setActiveRoutine();
                break;
            default:
                std::cout << "Invalid choice. Please try again." << std::endl;
        }

        if (!backToMainMenu) {
            std::cout << "\nPress Enter to continue...";
            std::cin.ignore();
            std::cin.get();

            // Clear screen (platform-specific)
            #ifdef _WIN32
                system("cls");
            #else
                system("clear");
            #endif
        }
    }
}

void GymCliApp::displayRoutinesMenu() {
    int width = 45;
    std::cout << std::string(width, '*') << std::endl;
    std::cout << "*" << std::setw(width - 2) << std::left << " GYM CLI - ROUTINE MANAGEMENT" << "*" << std::endl;
    std::cout << std::string(width, '*') << std::endl;
    std::cout << "1. View all routines" << std::endl;
    std::cout << "2. Add new routine" << std::endl;
    std::cout << "3. Edit routine" << std::endl;
    std::cout << "4. Delete routine" << std::endl;
    std::cout << "5. Set active routine" << std::endl;
    std::cout << "0. Back to main menu" << std::endl;
    std::cout << std::string(width, '*') << std::endl;
    std::cout << "Enter your choice: ";
}

void GymCliApp::viewAllRoutines() {
    const auto& routines = db.getAllRoutines();
    if (routines.empty()) {
        std::cout << "No routines found." << std::endl;
        return;
    }
    
    int width = 100; // Increased width to fit all days
    std::cout << std::string(width, '=') << std::endl;
    std::cout << std::setw(6) << "ID" 
              << std::setw(25) << "Routine Name"
              << std::setw(69) << "Day Assignments" << std::endl;
    std::cout << std::string(width, '-') << std::endl;
    
    for (size_t i = 0; i < routines.size(); i++) {
        const auto& routine = routines[i];
        bool isActive = (db.getActiveRoutine() == &routine);
        
        // Create a string for day assignments
        // Keep them in a specific order
        std::vector<std::string> days = {
            "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"
        };
        
        std::string dayAssignments;
        for (const auto& day : days) {
            BodyPart bodyPart = routine.getBodyPartForDay(day);
            std::string bodyPartStr;
            
            switch (bodyPart) {
                case BodyPart::UPPER: bodyPartStr = "Upper"; break;
                case BodyPart::LOWER: bodyPartStr = "Lower"; break;
                case BodyPart::FULL: bodyPartStr = "Full"; break;
                default: bodyPartStr = "Rest"; break;
            }
            
            if (!dayAssignments.empty()) {
                dayAssignments += ", ";
            }
            dayAssignments += day.substr(0, 3) + ":" + bodyPartStr;
        }
        
        // Include active marker if this is the active routine
        std::string nameDisplay = routine.getName();
        if (isActive) {
            nameDisplay += " (Active)";
        }
        
        std::cout << std::setw(6) << (i + 1)
                  << std::setw(25) << nameDisplay
                  << std::setw(69) << dayAssignments << std::endl;
    }
    
    std::cout << std::string(width, '=') << std::endl;
}

void GymCliApp::addNewRoutine() {
    std::string name;
    std::cout << "Enter routine name: ";
    std::cin.ignore();
    std::getline(std::cin, name);

    WorkoutRoutine routine(name);

    // Setup day assignments
    editRoutineDayAssignments(routine);

    // Add the routine
    db.addRoutine(routine);
    std::cout << "Routine added successfully!" << std::endl;
}

void GymCliApp::editRoutine() {
    const auto& routines = db.getAllRoutines();
    if (routines.empty()) {
        std::cout << "No routines found to edit." << std::endl;
        return;
    }

    viewAllRoutines();

    size_t routineId;
    std::cout << "Enter routine ID to edit (0 to cancel): ";
    std::cin >> routineId;

    if (routineId == 0 || routineId > routines.size()) {
        return;
    }

 // Get a copy of the routine to edit
    WorkoutRoutine editedRoutine = routines[routineId - 1];

    // Edit menu
    int width = 45;
    std::cout << std::string(width, '*') << std::endl;
    std::cout << "*" << std::setw(width - 2) << std::left << " EDIT ROUTINE: " + editedRoutine.getName() << "*" << std::endl;
    std::cout << std::string(width, '*') << std::endl;
    std::cout << "1. Edit name" << std::endl;
    std::cout << "2. Edit day assignments" << std::endl;
    std::cout << "0. Cancel" << std::endl;
    std::cout << std::string(width, '*') << std::endl;
    std::cout << "Enter your choice: ";

    int choice;
    std::cin >> choice;

    switch (choice) {
        case 1:
            editRoutineName(editedRoutine);
            break;
        case 2:
            editRoutineDayAssignments(editedRoutine);
            break;
        default:
            return;
    }

    // Update the routine
    db.updateRoutine(routineId - 1, editedRoutine);
    std::cout << "Routine updated successfully!" << std::endl;
}

void GymCliApp::deleteRoutine() {
    const auto& routines = db.getAllRoutines();
    if (routines.empty()) {
        std::cout << "No routines found to delete." << std::endl;
        return;
    }

    viewAllRoutines();

    size_t routineId;
    std::cout << "Enter routine ID to delete (0 to cancel): ";
    std::cin >> routineId;

    if (routineId == 0 || routineId > routines.size()) {
        return;
    }

    // Confirm deletion
    char confirm;
    std::cout << "Are you sure you want to delete this routine? [y/N]: ";
    std::cin >> confirm;

    if (tolower(confirm) != 'y') {
        std::cout << "Deletion cancelled." << std::endl;
        return;
    }

    // Delete the routine
    if (db.deleteRoutine(routineId - 1)) {
        std::cout << "Routine deleted successfully!" << std::endl;
    } else {
        std::cout << "Failed to delete routine." << std::endl;
    }
}

void GymCliApp::setActiveRoutine() {
    const auto& routines = db.getAllRoutines();
    if (routines.empty()) {
        std::cout << "No routines found." << std::endl;
        return;
    }

    viewAllRoutines();

    size_t routineId;
    std::cout << "Enter routine ID to set as active (0 to cancel): ";
    std::cin >> routineId;

    if (routineId == 0 || routineId > routines.size()) {
        return;
    }

    // Set as active
    if (db.setActiveRoutine(routineId - 1)) {
        std::cout << "Active routine set to: " << routines[routineId - 1].getName() << std::endl;
    } else {
        std::cout << "Failed to set active routine." << std::endl;
    }
}

void GymCliApp::editRoutineName(WorkoutRoutine& routine) {
    std::string newName;
    std::cout << "Enter new routine name: ";
    std::cin.ignore();
    std::getline(std::cin, newName);

    routine.setName(newName);
}

void GymCliApp::editRoutineDayAssignments(WorkoutRoutine& routine) {
    std::vector<std::string> days = {
        "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"
    };

    std::cout << "Assign body parts to each day of the week:" << std::endl;
    std::cout << "Options: (U)pper Body, (L)ower Body, (F)ull Body, (R)est Day" << std::endl;
    std::cout << std::endl;

    for (const auto& day : days) {
        BodyPart currentBodyPart = routine.getBodyPartForDay(day);
        char currentChoice = 'R'; // Default to rest

        switch (currentBodyPart) {
            case BodyPart::UPPER: currentChoice = 'U'; break;
            case BodyPart::LOWER: currentChoice = 'L'; break;
            case BodyPart::FULL: currentChoice = 'F'; break;
            default: currentChoice = 'R'; break;
        }

        std::cout << day << " [U/L/F/R] (current: " << currentChoice << "): ";
        char choice;
        std::cin >> choice;

        BodyPart bodyPart;
        switch (toupper(choice)) {
            case 'U': bodyPart = BodyPart::UPPER; break;
            case 'L': bodyPart = BodyPart::LOWER; break;
            case 'F': bodyPart = BodyPart::FULL; break;
            default: bodyPart = BodyPart::OTHER; break; // Rest day
        }

        routine.assignDayToBodyPart(day, bodyPart);
    }
}

void GymCliApp::run() {
    while (running) {
        displayMenu();
        int choice;
        std::cin >> choice;

        switch (choice) {
            case 0:
                running = false;
                break;
            case 1:
                addNewExercise();
                break;
            case 2:
                viewAllExercises();
                break;
            case 3:
                viewExercisesByCategory();
                break;
            case 4:
                viewExercisesByName();
                break;
            case 5:
                viewExercisesByDay();
                break;
            case 6:
                viewExercisesByBodyPart();
                break;
            case 7:
                viewExercisesByRoutine();
                break;
            case 8:
                viewWorkoutSessions();
                break;
            case 9:
                viewProgressForExercise();
                break;
            case 10:
                manageRoutines();
                break;
            default:
                std::cout << "Invalid choice. Please try again." << std::endl;
        }

        if (running) {
            std::cout << "\nPress Enter to continue...";
            std::cin.ignore();
            std::cin.get();

            // Clear screen (platform-specific)
            #ifdef _WIN32
                system("cls");
            #else
                system("clear");
            #endif
        }
    }

    std::cout << "Thank you for using GymCli. Goodbye!" << std::endl;
}