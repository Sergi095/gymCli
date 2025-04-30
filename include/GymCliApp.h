#ifndef GYM_CLI_APP_H
#define GYM_CLI_APP_H

#include "GymDatabse.h"

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

    // Helper method for determining body part from category
    BodyPart getCategoryBodyPart(const std::string& category);

public:
    GymCliApp();
    void run();
};

#endif // GYM_CLI_APP_H
