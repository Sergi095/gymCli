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
    void viewAllRoutines(bool offerDetails = true);
    void displayRoutineDetails(const WorkoutRoutine& routine);
    void addNewRoutine();
    void editRoutine();
    void deleteRoutine();
    void setActiveRoutine();
    void importRoutineFromCsv();


    void editRoutineName(WorkoutRoutine& routine);
    void editRoutineDayAssignments(WorkoutRoutine& routine);

    BodyPart getCategoryBodyPart(const std::string& category);
    bool isUpperBodyCategory(const std::string& category);
    bool isLowerBodyCategory(const std::string& category);

    std::vector<std::string> getDaysOfWeek();

public:
    GymCliApp();
    void run();
    bool importRoutineCsvFile(const std::string& filename, bool replaceExisting = false);
};

#endif // GYM_CLI_APP_H
