#include "GymCliApp.h"
#include "TableRenderer.h"
#include "Utils.h"
#include <iostream>
#include <iomanip>
#include <limits> // For std::numeric_limits

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

BodyPart GymCliApp::getCategoryBodyPart(const std::string& category) {
    if (isUpperBodyCategory(category)) {
        return BodyPart::UPPER;
    } else if (isLowerBodyCategory(category)) {
        return BodyPart::LOWER;
    } else if (containsIgnoreCase(category, "full") ||
              containsIgnoreCase(category, "cardio") ||
              containsIgnoreCase(category, "core")) {
        return BodyPart::FULL;
    } else {
        return BodyPart::OTHER;
    }
}

void GymCliApp::addNewExercise() {
    std::string name, category, notes;

    std::cout << "Enter exercise name: ";
    std::cin.ignore();
    std::getline(std::cin, name);

    // Get today's body part from active routine
    std::string todayDayOfWeek = getCurrentDayOfWeek();
    BodyPart todayBodyPart = db.getBodyPartForDay(todayDayOfWeek);
    std::string bodyPartStr;
    
    switch(todayBodyPart) {
        case BodyPart::UPPER: bodyPartStr = "Upper Body"; break;
        case BodyPart::LOWER: bodyPartStr = "Lower Body"; break;
        case BodyPart::FULL: bodyPartStr = "Full Body"; break;
        default: bodyPartStr = "Rest Day"; break;
    }
    
    // Ask which body part the exercise belongs to
    std::cout << "Today is " << todayDayOfWeek << " - Scheduled for: " << bodyPartStr << std::endl;
    char bodyPartChoice;
    BodyPart chosenBodyPart;
    
    std::cout << "Which body part does this exercise train?" << std::endl;
    std::cout << "U - Upper body (chest, back, shoulders, arms)" << std::endl;
    std::cout << "L - Lower body (legs, quads, hamstrings, calves)" << std::endl;
    std::cout << "F - Full body (compound exercises, core, cardio)" << std::endl;
    std::cout << "O - Other" << std::endl;
    
    // Default to today's scheduled body part
    std::cout << "Select body part [U/L/F/O] (default: ";
    switch(todayBodyPart) {
        case BodyPart::UPPER: std::cout << "U"; break;
        case BodyPart::LOWER: std::cout << "L"; break;
        case BodyPart::FULL: std::cout << "F"; break;
        default: std::cout << "O"; break;
    }
    std::cout << "): ";
    
    std::string choiceInput;
    std::getline(std::cin, choiceInput);
    
    if (choiceInput.empty()) {
        // Use default (today's body part)
        chosenBodyPart = todayBodyPart;
        bodyPartChoice = ' '; // placeholder
    } else {
        bodyPartChoice = choiceInput[0];
        switch (toupper(bodyPartChoice)) {
            case 'U': chosenBodyPart = BodyPart::UPPER; break;
            case 'L': chosenBodyPart = BodyPart::LOWER; break;
            case 'F': chosenBodyPart = BodyPart::FULL; break;
            default: chosenBodyPart = BodyPart::OTHER; break;
        }
    }
    
    // Suggest categories based on chosen body part
    switch(chosenBodyPart) {
        case BodyPart::UPPER:
            std::cout << "Suggested categories: Chest, Back, Shoulders, Arms, Biceps, Triceps" << std::endl;
            break;
        case BodyPart::LOWER:
            std::cout << "Suggested categories: Legs, Quads, Hamstrings, Calves, Glutes" << std::endl;
            break;
        case BodyPart::FULL:
            std::cout << "Suggested categories: Full Body, Core, Cardio, Compound" << std::endl;
            break;
        default:
            std::cout << "Suggested categories: Mobility, Flexibility, Recovery" << std::endl;
            break;
    }

    std::cout << "Enter category (e.g., Chest, Back, Legs): ";
    std::getline(std::cin, category);
    
    // Ask for routine association
    const WorkoutRoutine* activeRoutine = db.getActiveRoutine();
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

    // Ask for measurement type
    char typeChoice;
    std::cout << "Is this a rep-based exercise (R) or time-based exercise (T)? [R/T]: ";
    std::string typeChoiceStr;
    std::getline(std::cin, typeChoiceStr);
    typeChoice = typeChoiceStr.empty() ? 'R' : typeChoiceStr[0];

    MeasurementType measureType = (toupper(typeChoice) == 'T') ?
                                  MeasurementType::TIME :
                                  MeasurementType::REPS;

    // Create exercise with today's actual date
    Exercise exercise(name, category, measureType, chosenBodyPart);
    // The date is set to today's date by the constructor
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

    // Validate input to prevent infinite loops
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
                std::cout << "Invalid number of reps. Setting to 1." << std::endl;
                reps = 1;
            }

            // Validate reps input
            if (reps <= 0 || reps > 1000) {
                std::cout << "Invalid number of reps. Setting to 1." << std::endl;
                reps = 1;
            }

            std::cout << "Set " << (i + 1) << " - Enter weight (kg): ";
            std::string weightStr;
            std::getline(std::cin, weightStr);
            try {
                weight = std::stod(weightStr);
            } catch (...) {
                std::cout << "Invalid weight. Setting to 0.0." << std::endl;
                weight = 0.0;
            }

            // Validate weight input
            if (weight < 0 || weight > 1000) {
                std::cout << "Invalid weight. Setting to 0.0." << std::endl;
                weight = 0.0;
            }

            exercise.addRepSet(reps, weight);
        } else {
            // Improved time input method
            char timeFormat;
            std::cout << "Set " << (i + 1) << " - Select time format:" << std::endl;
            std::cout << "  S - Enter time in seconds only" << std::endl;
            std::cout << "  M - Enter time in minutes and seconds" << std::endl;
            std::cout << "Format [S/M]: ";
            std::string formatStr;
            std::getline(std::cin, formatStr);
            timeFormat = formatStr.empty() ? 'S' : formatStr[0];
            
            int totalSeconds = 0;
            
            if (toupper(timeFormat) == 'M') {
                int minutes, seconds;
                
                std::cout << "Set " << (i + 1) << " - Enter duration (minutes): ";
                std::string minutesStr;
                std::getline(std::cin, minutesStr);
                try {
                    minutes = std::stoi(minutesStr);
                } catch (...) {
                    std::cout << "Invalid minutes. Setting to 0." << std::endl;
                    minutes = 0;
                }

                // Validate minutes input
                if (minutes < 0 || minutes > 60) {
                    std::cout << "Invalid minutes. Setting to 0." << std::endl;
                    minutes = 0;
                }

                std::cout << "Set " << (i + 1) << " - Enter duration (seconds): ";
                std::string secondsStr;
                std::getline(std::cin, secondsStr);
                try {
                    seconds = std::stoi(secondsStr);
                } catch (...) {
                    std::cout << "Invalid seconds. Setting to 0." << std::endl;
                    seconds = 0;
                }

                // Validate seconds input
                if (seconds < 0 || seconds > 59) {
                    std::cout << "Invalid seconds. Setting to 0." << std::endl;
                    seconds = 0;
                }

                totalSeconds = minutes * 60 + seconds;
            } else {
                std::cout << "Set " << (i + 1) << " - Enter duration (total seconds): ";
                std::string secondsStr;
                std::getline(std::cin, secondsStr);
                try {
                    totalSeconds = std::stoi(secondsStr);
                } catch (...) {
                    std::cout << "Invalid seconds. Setting to 0." << std::endl;
                    totalSeconds = 0;
                }
                
                // Validate seconds input
                if (totalSeconds < 0 || totalSeconds > 3600) {
                    std::cout << "Invalid seconds. Setting to 0." << std::endl;
                    totalSeconds = 0;
                }
            }

            // Ensure at least 1 second
            if (totalSeconds <= 0) {
                std::cout << "Duration must be at least 1 second. Setting to 1 second." << std::endl;
                totalSeconds = 1;
            }

            std::cout << "Set " << (i + 1) << " - Enter weight (kg): ";
            std::string weightStr;
            std::getline(std::cin, weightStr);
            try {
                weight = std::stod(weightStr);
            } catch (...) {
                std::cout << "Invalid weight. Setting to 0.0." << std::endl;
                weight = 0.0;
            }

            // Validate weight input
            if (weight < 0 || weight > 1000) {
                std::cout << "Invalid weight. Setting to 0.0." << std::endl;
                weight = 0.0;
            }

            exercise.addTimeSet(totalSeconds, weight);
        }
    }

    std::cout << "Enter notes (optional, press Enter to skip): ";
    std::getline(std::cin, notes);
    exercise.setNotes(notes);

    db.addExercise(exercise);
    std::cout << "Exercise added successfully!" << std::endl;

    // Display the added exercise
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

    const auto& exercises = db.getExercisesByDay(dayOfWeek);
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
