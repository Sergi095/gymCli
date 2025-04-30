#ifndef WORKOUT_ROUTINE_H
#define WORKOUT_ROUTINE_H

#include <string>
#include <vector>
#include <map>
#include "Exercise.h"

class WorkoutRoutine {
private:
    std::string name;
    std::map<std::string, BodyPart> dayAssignments; // Maps days to body parts
public:
    WorkoutRoutine(const std::string& routineName);

    // Getters and setters
    std::string getName() const;
    void setName(const std::string& newName);

    // Day assignment methods
    void assignDayToBodyPart(const std::string& day, BodyPart bodyPart);
    BodyPart getBodyPartForDay(const std::string& day) const;
    bool isDayAssigned(const std::string& day) const;
    void clearDayAssignment(const std::string& day);

    // Get all day assignments
    std::map<std::string, BodyPart> getDayAssignments() const;

    // Serialization
    std::string serialize() const;
    static WorkoutRoutine deserialize(const std::string& data);
};

#endif // WORKOUT_ROUTINE_H

