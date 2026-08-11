#ifndef WORKOUT_ROUTINE_H
#define WORKOUT_ROUTINE_H

#include <string>
#include <vector>
#include <map>
#include "Exercise.h"

struct RoutineExercise {
    std::string section;
    std::string name;
    std::string sets;
    std::string reps;
    std::string duration;
    std::string weight;
    std::string rest;
    std::string notes;
    std::string link;
};

class WorkoutRoutine {
private:
    std::string name;
    std::string focus;
    std::string notes;
    std::map<std::string, BodyPart> dayAssignments; // Maps days to body parts
    std::vector<RoutineExercise> exercises;
public:
    WorkoutRoutine(const std::string& routineName);

    // Getters and setters
    std::string getName() const;
    void setName(const std::string& newName);
    std::string getNotes() const;
    void setNotes(const std::string& newNotes);
    std::string getFocus() const;
    void setFocus(const std::string& newFocus);

    // Day assignment methods
    void assignDayToBodyPart(const std::string& day, BodyPart bodyPart);
    BodyPart getBodyPartForDay(const std::string& day) const;
    bool isDayAssigned(const std::string& day) const;
    void clearDayAssignment(const std::string& day);

    // Get all day assignments
    std::map<std::string, BodyPart> getDayAssignments() const;

    void addExercise(const RoutineExercise& exercise);
    const std::vector<RoutineExercise>& getExercises() const;

    // Serialization
    std::string serialize() const;
    static WorkoutRoutine deserialize(const std::string& data);
};

#endif // WORKOUT_ROUTINE_H
