#ifndef GYM_DATABASE_H
#define GYM_DATABASE_H

#include <vector>
#include <map>
#include <string>
#include "Exercise.h"
#include "WorkoutRoutine.h"

class GymDatabase {
private:
    std::vector<Exercise> exercises;
    std::vector<WorkoutRoutine> routines;
    std::string dbFilename;
    std::string routinesFilename;

    // Currently active routine
    int activeRoutineIndex;

public:
    GymDatabase(const std::string& filename = "gym_data.db",
                const std::string& routinesFile = "gym_routines.db");

    // Exercise methods
    void addExercise(const Exercise& exercise);
    const std::vector<Exercise>& getAllExercises() const;
    std::vector<Exercise> getExercisesByName(const std::string& name) const;
    std::vector<Exercise> getExercisesByCategory(const std::string& category) const;
    std::vector<Exercise> getExercisesByDay(const std::string& dayOfWeek) const;
    std::vector<Exercise> getExercisesByBodyPart(BodyPart bodyPart) const;
    std::vector<Exercise> getExercisesByRoutine(const std::string& routineName) const;
    std::map<std::string, std::vector<Exercise>> getSessionMap() const;
    std::map<std::string, double> getProgressData(const std::string& exerciseName) const;

    // Routine methods
    void addRoutine(const WorkoutRoutine& routine);
    bool updateRoutine(size_t index, const WorkoutRoutine& routine);
    bool deleteRoutine(size_t index);
    const std::vector<WorkoutRoutine>& getAllRoutines() const;
    const WorkoutRoutine* getActiveRoutine() const;
    bool setActiveRoutine(size_t index);
    BodyPart getBodyPartForToday() const;
    BodyPart getBodyPartForDay(const std::string& day) const;

    // File I/O methods
    void saveToFile() const;
    void loadFromFile();
    void saveRoutinesToFile() const;
    void loadRoutinesFromFile();
};

#endif // GYM_DATABASE_H
