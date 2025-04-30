#ifndef GYM_CLI_APP_H
#define GYM_CLI_APP_H

#include "GymDatabse.h"
#include <string>
#include <vector>

class GymCliApp {
private:
    GymDatabase db;
    bool running;

    void displayMenu();
    void addNewExercise();
    void viewAllExercises();
    void viewExercisesByCategory();
    void viewExercisesByName();
    void viewExercisesByDay();
    void viewExercisesByBodyPart();
    void viewExercisesByRoutine();
    void viewWorkoutSessions();
    void viewProgressForExercise();

    // Routine management methods
    void manageRoutines();
    void displayRoutinesMenu();
    void viewAllRoutines();
    void addNewRoutine();
    void editRoutine();
    void deleteRoutine();
    void setActiveRoutine();

    // Helper methods for routine editing
    void editRoutineName(WorkoutRoutine& routine);
    void editRoutineDayAssignments(WorkoutRoutine& routine);

    // Helper methods for determining body part from category
    BodyPart getCategoryBodyPart(const std::string& category);
    bool isUpperBodyCategory(const std::string& category);
    bool isLowerBodyCategory(const std::string& category);
    bool containsIgnoreCase(const std::string& str, const std::string& substr);
    
    // Helper methods for dates and days
    std::vector<std::string> getDaysOfWeek();
    std::string getCurrentDayOfWeek();
    std::string getCurrentDate();
    std::string getDateForDayOfWeek(const std::string& dayOfWeek);

public:
    GymCliApp();
    void run();
};

#endif // GYM_CLI_APP_H