#include "GymCliApp.h"
#include "TableRenderer.h"
#include "Utils.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <limits>
#include <cctype>
#include <sstream>
#include <iterator>

namespace {

const int NO_DEFAULT = std::numeric_limits<int>::min();

std::string shortened(const std::string& value, size_t width) {
    if (value.size() <= width) {
        return value;
    }
    if (width <= 3) {
        return value.substr(0, width);
    }
    return value.substr(0, width - 3) + "...";
}

std::string storedText(const std::string& value) {
    std::string result = trim(value);
    std::replace(result.begin(), result.end(), '|', '/');
    return result;
}

bool promptLine(const std::string& prompt, std::string& value) {
    std::cout << prompt;
    if (!std::getline(std::cin, value)) {
        return false;
    }
    value = trim(value);
    return true;
}

bool parseInt(const std::string& value, int& result) {
    std::istringstream input(value);
    char trailing = '\0';
    return (input >> result) && !(input >> trailing);
}

bool parseDouble(const std::string& value, double& result) {
    std::istringstream input(value);
    char trailing = '\0';
    return (input >> result) && !(input >> trailing);
}

bool promptInt(const std::string& prompt, int minimum, int maximum,
               int& result, int defaultValue = NO_DEFAULT) {
    while (true) {
        std::string value;
        if (!promptLine(prompt, value)) {
            return false;
        }
        if (value.empty() && defaultValue != NO_DEFAULT) {
            result = defaultValue;
            return true;
        }
        if (parseInt(value, result) && result >= minimum && result <= maximum) {
            return true;
        }
        std::cout << "Please enter a number from " << minimum << " to " << maximum << ".\n";
    }
}

bool promptDouble(const std::string& prompt, double minimum, double maximum,
                  double& result, double defaultValue) {
    while (true) {
        std::string value;
        if (!promptLine(prompt, value)) {
            return false;
        }
        if (value.empty()) {
            result = defaultValue;
            return true;
        }
        if (parseDouble(value, result) && result >= minimum && result <= maximum) {
            return true;
        }
        std::cout << "Please enter a value from " << minimum << " to " << maximum << ".\n";
    }
}

bool promptChoice(const std::string& prompt, const std::string& allowed,
                  char& result, char defaultValue = '\0') {
    while (true) {
        std::string value;
        if (!promptLine(prompt, value)) {
            return false;
        }
        if (value.empty() && defaultValue != '\0') {
            result = static_cast<char>(std::toupper(static_cast<unsigned char>(defaultValue)));
            return true;
        }
        if (value.size() == 1) {
            const char choice = static_cast<char>(std::toupper(static_cast<unsigned char>(value[0])));
            if (allowed.find(choice) != std::string::npos) {
                result = choice;
                return true;
            }
        }
        std::cout << "Choose one of: " << allowed << ".\n";
    }
}

bool promptYesNo(const std::string& prompt, bool defaultValue, bool& result) {
    char choice = '\0';
    if (!promptChoice(prompt, "YN", choice, defaultValue ? 'Y' : 'N')) {
        return false;
    }
    result = choice == 'Y';
    return true;
}

bool promptRequired(const std::string& prompt, std::string& result) {
    while (true) {
        if (!promptLine(prompt, result)) {
            return false;
        }
        result = storedText(result);
        if (!result.empty()) {
            return true;
        }
        std::cout << "This field cannot be empty.\n";
    }
}

bool parseDuration(const std::string& value, int& seconds) {
    const size_t colon = value.find(':');
    if (colon == std::string::npos) {
        return parseInt(value, seconds) && seconds > 0 && seconds <= 86400;
    }
    if (value.find(':', colon + 1) != std::string::npos) {
        return false;
    }

    int minutes = 0;
    int remainingSeconds = 0;
    if (!parseInt(value.substr(0, colon), minutes) ||
        !parseInt(value.substr(colon + 1), remainingSeconds) ||
        minutes < 0 || remainingSeconds < 0 || remainingSeconds > 59) {
        return false;
    }
    seconds = minutes * 60 + remainingSeconds;
    return seconds > 0 && seconds <= 86400;
}

bool promptDuration(const std::string& prompt, int& seconds) {
    while (true) {
        std::string value;
        if (!promptLine(prompt, value)) {
            return false;
        }
        if (parseDuration(value, seconds)) {
            return true;
        }
        std::cout << "Use seconds or mm:ss (for example, 90 or 1:30).\n";
    }
}

std::string bodyPartName(BodyPart bodyPart, bool restLabel = false) {
    switch (bodyPart) {
        case BodyPart::UPPER: return "Upper Body";
        case BodyPart::LOWER: return "Lower Body";
        case BodyPart::FULL: return "Full Body";
        default: return restLabel ? "Rest Day" : "Other";
    }
}

char bodyPartCode(BodyPart bodyPart) {
    switch (bodyPart) {
        case BodyPart::UPPER: return 'U';
        case BodyPart::LOWER: return 'L';
        case BodyPart::FULL: return 'F';
        default: return 'O';
    }
}

BodyPart bodyPartFromCode(char choice) {
    switch (choice) {
        case 'U': return BodyPart::UPPER;
        case 'L': return BodyPart::LOWER;
        case 'F': return BodyPart::FULL;
        default: return BodyPart::OTHER;
    }
}

void boxLine(const std::string& value, int width) {
    const std::string content = shortened(value, static_cast<size_t>(width - 4));
    std::cout << "| " << std::left << std::setw(width - 4) << content << " |\n";
}

void waitForEnter() {
    if (!isInteractiveInput()) {
        return;
    }
    std::cout << "\nPress Enter to continue...";
    std::string ignored;
    std::getline(std::cin, ignored);
}

void offerExerciseDetails(const std::vector<Exercise>& exercises) {
    if (exercises.empty()) {
        return;
    }
    int id = 0;
    if (promptInt("View exercise ID [0 to return]: ", 0,
                  static_cast<int>(exercises.size()), id, 0) && id > 0) {
        TableRenderer::renderExercise(exercises[static_cast<size_t>(id - 1)]);
    }
}

} // namespace

GymCliApp::GymCliApp() : running(true) {}

void GymCliApp::displayMenu() {
    const int width = std::min(getTerminalWidth(), 60);
    const WorkoutRoutine* activeRoutine = db.getActiveRoutine();

    std::cout << '\n' << std::string(width, '=') << '\n';
    boxLine("GYMCLI", width);
    boxLine(::getCurrentDate() + " - " + ::getCurrentDayOfWeek(), width);
    boxLine("Today: " + bodyPartName(db.getBodyPartForToday(), true), width);
    if (activeRoutine != nullptr) {
        boxLine("Routine: " + activeRoutine->getName(), width);
    }
    std::cout << std::string(width, '=') << "\n\n";
    std::cout << "Workout\n"
              << "  [1] Log an exercise\n"
              << "  [2] Exercise history\n"
              << "  [8] Sessions by date\n"
              << "  [9] Exercise progress\n\n"
              << "Find\n"
              << "  [3] By category\n"
              << "  [4] By exercise name\n"
              << "  [5] By weekday\n"
              << "  [6] By body part\n"
              << "  [7] By routine\n\n"
              << "Setup\n"
              << "  [10] Manage routines\n"
              << "  [0] Exit\n\n";
}

bool GymCliApp::isUpperBodyCategory(const std::string& category) {
    const std::vector<std::string> keywords = {
        "chest", "back", "shoulder", "arm", "bicep", "tricep", "push",
        "pull", "press", "curl", "delt", "trap", "lat", "upper"
    };
    for (const auto& keyword : keywords) {
        if (::containsIgnoreCase(category, keyword)) {
            return true;
        }
    }
    return false;
}

bool GymCliApp::isLowerBodyCategory(const std::string& category) {
    const std::vector<std::string> keywords = {
        "leg", "quad", "hamstring", "calf", "calves", "glute", "squat",
        "deadlift", "lunge", "hip", "thigh", "knee", "lower"
    };
    for (const auto& keyword : keywords) {
        if (::containsIgnoreCase(category, keyword)) {
            return true;
        }
    }
    return false;
}

BodyPart GymCliApp::getCategoryBodyPart(const std::string& category) {
    if (isUpperBodyCategory(category)) {
        return BodyPart::UPPER;
    }
    if (isLowerBodyCategory(category)) {
        return BodyPart::LOWER;
    }
    const std::vector<std::string> fullBodyKeywords = {
        "full", "cardio", "core", "abs", "hiit", "circuit", "functional"
    };
    for (const auto& keyword : fullBodyKeywords) {
        if (::containsIgnoreCase(category, keyword)) {
            return BodyPart::FULL;
        }
    }
    return BodyPart::OTHER;
}

std::vector<std::string> GymCliApp::getDaysOfWeek() {
    return {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
}

void GymCliApp::addNewExercise() {
    std::cout << "\nLog an exercise\n----------------\n";

    std::string name;
    std::string category;
    if (!promptRequired("Exercise name: ", name) ||
        !promptRequired("Category (Chest, Back, Legs, ...): ", category)) {
        return;
    }

    BodyPart exerciseBodyPart = getCategoryBodyPart(category);
    const char detectedCode = bodyPartCode(exerciseBodyPart);
    char bodyChoice = detectedCode;
    const std::string bodyPrompt = "Body part [U/L/F/O, default " +
                                   std::string(1, detectedCode) + "]: ";
    if (!promptChoice(bodyPrompt, "ULFO", bodyChoice, detectedCode)) {
        return;
    }
    exerciseBodyPart = bodyPartFromCode(bodyChoice);

    const std::string today = ::getCurrentDate();
    std::string workoutDate;
    while (true) {
        if (!promptLine("Workout date [" + today + "]: ", workoutDate)) {
            return;
        }
        if (workoutDate.empty()) {
            workoutDate = today;
        }
        if (isValidDate(workoutDate)) {
            break;
        }
        std::cout << "Enter a real date in YYYY-MM-DD format.\n";
    }

    const WorkoutRoutine* activeRoutine = db.getActiveRoutine();
    std::string routineName;
    if (activeRoutine != nullptr) {
        const std::string workoutDay = getDayOfWeek(workoutDate);
        const BodyPart plannedBodyPart = db.getBodyPartForDay(workoutDay);
        std::cout << "Scheduled for " << workoutDay << ": "
                  << bodyPartName(plannedBodyPart, true) << ".\n";
        if (plannedBodyPart != BodyPart::OTHER && plannedBodyPart != exerciseBodyPart) {
            std::cout << "Note: this exercise differs from the routine's scheduled body part.\n";
        }

        bool associate = true;
        if (!promptYesNo("Add to routine '" + activeRoutine->getName() + "'? [Y/n]: ",
                         true, associate)) {
            return;
        }
        if (associate) {
            routineName = activeRoutine->getName();
        }
    }

    char typeChoice = 'R';
    if (!promptChoice("Tracking type [R]eps/[T]ime (default R): ", "RT", typeChoice, 'R')) {
        return;
    }
    const MeasurementType measureType = typeChoice == 'T'
        ? MeasurementType::TIME
        : MeasurementType::REPS;

    int numberOfSets = 3;
    if (!promptInt("Number of sets [3]: ", 1, 100, numberOfSets, 3)) {
        return;
    }

    Exercise exercise(name, category, measureType, exerciseBodyPart);
    exercise.setDate(workoutDate);
    exercise.setRoutineName(routineName);

    for (int index = 0; index < numberOfSets; ++index) {
        std::cout << "\nSet " << (index + 1) << " of " << numberOfSets << '\n';
        double weight = 0;
        if (measureType == MeasurementType::REPS) {
            int reps = 0;
            if (!promptInt("  Reps: ", 1, 1000, reps) ||
                !promptDouble("  Weight in kg [0]: ", 0, 2000, weight, 0)) {
                return;
            }
            exercise.addRepSet(reps, weight);
        } else {
            int seconds = 0;
            if (!promptDuration("  Duration (seconds or mm:ss): ", seconds) ||
                !promptDouble("  Weight in kg [0]: ", 0, 2000, weight, 0)) {
                return;
            }
            exercise.addTimeSet(seconds, weight);
        }
    }

    std::string notes;
    if (!promptLine("\nNotes [optional]: ", notes)) {
        return;
    }
    exercise.setNotes(storedText(notes));
    db.addExercise(exercise);

    std::cout << "\nWorkout saved.\n";
    TableRenderer::renderExercise(exercise);
}

void GymCliApp::viewAllExercises() {
    const auto& exercises = db.getAllExercises();
    TableRenderer::renderExerciseList(exercises);
    offerExerciseDetails(exercises);
}

void GymCliApp::viewExercisesByCategory() {
    std::string category;
    if (!promptRequired("Category search: ", category)) {
        return;
    }
    const auto exercises = db.getExercisesByCategory(category);
    TableRenderer::renderExerciseList(exercises);
    offerExerciseDetails(exercises);
}

void GymCliApp::viewExercisesByName() {
    std::string name;
    if (!promptRequired("Exercise name search: ", name)) {
        return;
    }
    const auto exercises = db.getExercisesByName(name);
    TableRenderer::renderExerciseList(exercises);
    offerExerciseDetails(exercises);
}

void GymCliApp::viewExercisesByDay() {
    const auto days = getDaysOfWeek();
    std::cout << "\nWeekday\n";
    for (size_t i = 0; i < days.size(); ++i) {
        std::cout << "  [" << (i + 1) << "] " << days[i] << '\n';
    }

    int dayId = 0;
    if (!promptInt("Choose weekday [0 to cancel]: ", 0, 7, dayId) || dayId == 0) {
        return;
    }
    const auto exercises = db.getExercisesByDay(days[static_cast<size_t>(dayId - 1)]);
    TableRenderer::renderExerciseList(exercises);
    offerExerciseDetails(exercises);
}

void GymCliApp::viewExercisesByBodyPart() {
    char choice = 'U';
    if (!promptChoice("Body part [U]pper/[L]ower/[F]ull/[O]ther: ", "ULFO", choice)) {
        return;
    }
    const auto exercises = db.getExercisesByBodyPart(bodyPartFromCode(choice));
    TableRenderer::renderExerciseList(exercises);
    offerExerciseDetails(exercises);
}

void GymCliApp::viewExercisesByRoutine() {
    const auto& routines = db.getAllRoutines();
    if (routines.empty()) {
        std::cout << "No routines found.\n";
        return;
    }

    std::cout << "\nRoutines\n";
    for (size_t i = 0; i < routines.size(); ++i) {
        std::cout << "  [" << (i + 1) << "] " << routines[i].getName();
        if (db.getActiveRoutine() == &routines[i]) {
            std::cout << " (active)";
        }
        std::cout << '\n';
    }

    int routineId = 0;
    if (!promptInt("Choose routine [0 to cancel]: ", 0,
                   static_cast<int>(routines.size()), routineId) || routineId == 0) {
        return;
    }
    const auto exercises = db.getExercisesByRoutine(
        routines[static_cast<size_t>(routineId - 1)].getName());
    TableRenderer::renderExerciseList(exercises);
    offerExerciseDetails(exercises);
}

void GymCliApp::viewWorkoutSessions() {
    const auto sessionMap = db.getSessionMap();
    TableRenderer::renderSessionList(sessionMap);
    if (sessionMap.empty()) {
        return;
    }

    int sessionId = 0;
    if (!promptInt("View session number [0 to return]: ", 0,
                   static_cast<int>(sessionMap.size()), sessionId, 0) || sessionId == 0) {
        return;
    }
    auto selected = sessionMap.begin();
    std::advance(selected, sessionId - 1);
    TableRenderer::renderExerciseList(selected->second);
    offerExerciseDetails(selected->second);
}

void GymCliApp::viewProgressForExercise() {
    std::string name;
    if (!promptRequired("Exercise name: ", name)) {
        return;
    }
    TableRenderer::renderProgressChart(name, db.getProgressData(name));
}

void GymCliApp::manageRoutines() {
    while (running && std::cin.good()) {
        displayRoutinesMenu();
        int choice = 0;
        if (!promptInt("Choose an option: ", 0, 5, choice)) {
            return;
        }
        if (choice == 0) {
            clearTerminal();
            return;
        }

        switch (choice) {
            case 1: viewAllRoutines(); break;
            case 2: addNewRoutine(); break;
            case 3: editRoutine(); break;
            case 4: deleteRoutine(); break;
            case 5: setActiveRoutine(); break;
            default: break;
        }
        if (!std::cin.good()) {
            return;
        }
        waitForEnter();
        clearTerminal();
    }
}

void GymCliApp::displayRoutinesMenu() {
    const int width = std::min(getTerminalWidth(), 60);
    std::cout << '\n' << std::string(width, '=') << '\n';
    boxLine("ROUTINES", width);
    std::cout << std::string(width, '=') << "\n\n"
              << "  [1] View routines\n"
              << "  [2] Add routine\n"
              << "  [3] Edit routine\n"
              << "  [4] Delete routine\n"
              << "  [5] Set active routine\n"
              << "  [0] Back\n\n";
}

void GymCliApp::viewAllRoutines() {
    const auto& routines = db.getAllRoutines();
    if (routines.empty()) {
        std::cout << "No routines found.\n";
        return;
    }

    const auto days = getDaysOfWeek();
    const int width = getTerminalWidth();
    std::cout << "\n" << routines.size() << (routines.size() == 1 ? " routine\n" : " routines\n");
    for (size_t i = 0; i < routines.size(); ++i) {
        std::cout << std::string(width, '-') << '\n';
        std::string title = "[" + std::to_string(i + 1) + "] " + routines[i].getName();
        if (db.getActiveRoutine() == &routines[i]) {
            title += " (active)";
        }
        std::cout << shortened(title, width) << '\n';

        for (size_t start = 0; start < days.size(); start += 4) {
            std::string schedule = "    ";
            for (size_t index = start; index < std::min(start + 4, days.size()); ++index) {
                if (index > start) {
                    schedule += "  ";
                }
                const BodyPart bodyPart = routines[i].getBodyPartForDay(days[index]);
                const char code = bodyPart == BodyPart::OTHER ? 'R' : bodyPartCode(bodyPart);
                schedule += days[index].substr(0, 3) + ":" + code;
            }
            std::cout << shortened(schedule, width) << '\n';
        }
    }
    std::cout << std::string(width, '-') << '\n';
    std::cout << "U=Upper, L=Lower, F=Full, R=Rest\n";
}

void GymCliApp::addNewRoutine() {
    std::string name;
    if (!promptRequired("Routine name: ", name)) {
        return;
    }
    for (const auto& existing : db.getAllRoutines()) {
        if (equalsIgnoreCase(existing.getName(), name)) {
            std::cout << "A routine with that name already exists.\n";
            return;
        }
    }
    WorkoutRoutine routine(name);
    editRoutineDayAssignments(routine);
    if (!std::cin.good()) {
        return;
    }
    db.addRoutine(routine);
    std::cout << "Routine added.\n";
}

void GymCliApp::editRoutine() {
    const auto& routines = db.getAllRoutines();
    if (routines.empty()) {
        std::cout << "No routines found to edit.\n";
        return;
    }
    viewAllRoutines();

    int routineId = 0;
    if (!promptInt("Routine ID [0 to cancel]: ", 0,
                   static_cast<int>(routines.size()), routineId) || routineId == 0) {
        return;
    }

    WorkoutRoutine editedRoutine = routines[static_cast<size_t>(routineId - 1)];
    std::cout << "\n  [1] Rename\n  [2] Edit weekly schedule\n  [0] Cancel\n";
    int choice = 0;
    if (!promptInt("Choose an option: ", 0, 2, choice) || choice == 0) {
        return;
    }
    if (choice == 1) {
        editRoutineName(editedRoutine);
        for (size_t index = 0; index < routines.size(); ++index) {
            if (index != static_cast<size_t>(routineId - 1) &&
                equalsIgnoreCase(routines[index].getName(), editedRoutine.getName())) {
                std::cout << "A routine with that name already exists.\n";
                return;
            }
        }
    } else {
        editRoutineDayAssignments(editedRoutine);
    }
    if (!std::cin.good()) {
        return;
    }

    db.updateRoutine(static_cast<size_t>(routineId - 1), editedRoutine);
    std::cout << "Routine updated.\n";
}

void GymCliApp::deleteRoutine() {
    const auto& routines = db.getAllRoutines();
    if (routines.empty()) {
        std::cout << "No routines found to delete.\n";
        return;
    }
    viewAllRoutines();

    int routineId = 0;
    if (!promptInt("Routine ID [0 to cancel]: ", 0,
                   static_cast<int>(routines.size()), routineId) || routineId == 0) {
        return;
    }
    bool confirmed = false;
    if (!promptYesNo("Delete '" + routines[static_cast<size_t>(routineId - 1)].getName() +
                     "'? [y/N]: ", false, confirmed) || !confirmed) {
        std::cout << "Deletion cancelled.\n";
        return;
    }
    if (db.deleteRoutine(static_cast<size_t>(routineId - 1))) {
        std::cout << "Routine deleted.\n";
    }
}

void GymCliApp::setActiveRoutine() {
    const auto& routines = db.getAllRoutines();
    if (routines.empty()) {
        std::cout << "No routines found.\n";
        return;
    }
    viewAllRoutines();

    int routineId = 0;
    if (!promptInt("Routine ID [0 to cancel]: ", 0,
                   static_cast<int>(routines.size()), routineId) || routineId == 0) {
        return;
    }
    if (db.setActiveRoutine(static_cast<size_t>(routineId - 1))) {
        std::cout << "Active routine: "
                  << routines[static_cast<size_t>(routineId - 1)].getName() << '\n';
    }
}

void GymCliApp::editRoutineName(WorkoutRoutine& routine) {
    std::string newName;
    if (promptRequired("New routine name: ", newName)) {
        routine.setName(newName);
    }
}

void GymCliApp::editRoutineDayAssignments(WorkoutRoutine& routine) {
    std::cout << "\nWeekly schedule\n"
              << "U=Upper, L=Lower, F=Full, R=Rest. Press Enter to keep the current value.\n";

    for (const auto& day : getDaysOfWeek()) {
        const BodyPart current = routine.getBodyPartForDay(day);
        const char currentCode = current == BodyPart::OTHER ? 'R' : bodyPartCode(current);
        char choice = currentCode;
        if (!promptChoice("  " + day + " [" + currentCode + "]: ",
                          "ULFR", choice, currentCode)) {
            return;
        }
        routine.assignDayToBodyPart(day, choice == 'R'
            ? BodyPart::OTHER
            : bodyPartFromCode(choice));
    }
}

void GymCliApp::run() {
    while (running && std::cin.good()) {
        displayMenu();
        int choice = 0;
        if (!promptInt("Choose an option: ", 0, 10, choice)) {
            break;
        }

        bool pauseAfterAction = true;
        switch (choice) {
            case 0: running = false; pauseAfterAction = false; break;
            case 1: addNewExercise(); break;
            case 2: viewAllExercises(); break;
            case 3: viewExercisesByCategory(); break;
            case 4: viewExercisesByName(); break;
            case 5: viewExercisesByDay(); break;
            case 6: viewExercisesByBodyPart(); break;
            case 7: viewExercisesByRoutine(); break;
            case 8: viewWorkoutSessions(); break;
            case 9: viewProgressForExercise(); break;
            case 10: manageRoutines(); pauseAfterAction = false; break;
            default: break;
        }

        if (!std::cin.good()) {
            break;
        }
        if (running && pauseAfterAction) {
            waitForEnter();
            clearTerminal();
        }
    }
    std::cout << "\nSee you next workout.\n";
}
