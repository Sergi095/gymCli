#ifndef WORKOUT_SESSION_H
#define WORKOUT_SESSION_H

#include <string>
#include <vector>
#include "Exercise.h"

class WorkoutSession {
private:
    std::string date;
    std::vector<Exercise> exercises;

public:
    WorkoutSession();

    void addExercise(const Exercise& exercise);

    const std::vector<Exercise>& getExercises() const;
    std::string getDate() const;
};

#endif // WORKOUT_SESSION_H
