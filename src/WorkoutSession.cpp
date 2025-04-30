#include "WorkoutSession.h"
#include "Utils.h"

WorkoutSession::WorkoutSession() : date(getCurrentDate()) {}

void WorkoutSession::addExercise(const Exercise& exercise) {
    exercises.push_back(exercise);
}

const std::vector<Exercise>& WorkoutSession::getExercises() const {
    return exercises;
}

std::string WorkoutSession::getDate() const {
    return date;
}
