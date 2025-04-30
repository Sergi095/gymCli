#ifndef EXERCISE_H
#define EXERCISE_H

#include <string>
#include <vector>

enum class MeasurementType {
    REPS,
    TIME
};

enum class BodyPart {
    UPPER,
    LOWER,
    FULL,
    OTHER
};

class Exercise {
private:
    std::string name;
    std::string category;
    std::string date;
    std::string notes;
    MeasurementType measurementType;
    BodyPart bodyPart;
    std::string routineName; // Name of the routine this exercise is part of

    std::vector<int> sets;
    std::vector<int> reps;
    std::vector<int> duration;
    std::vector<double> weights;

public:
    Exercise(const std::string& n = "", const std::string& cat = "",
             MeasurementType type = MeasurementType::REPS,
             BodyPart bp = BodyPart::OTHER);

    void addRepSet(int setReps, double weight);
    void addTimeSet(int setSeconds, double weight);
    void setNotes(const std::string& n);
    void setBodyPart(BodyPart bp);
    void setRoutineName(const std::string& name);
    void setDate(const std::string& d);

    std::string getName() const;
    std::string getCategory() const;
    std::string getDate() const;
    std::string getNotes() const;
    MeasurementType getMeasurementType() const;
    BodyPart getBodyPart() const;
    std::string getBodyPartString() const;
    std::string getRoutineName() const;

    const std::vector<int>& getSets() const;
    const std::vector<int>& getReps() const;
    const std::vector<int>& getDuration() const;
    const std::vector<double>& getWeights() const;

    double calculateVolume() const;

    static std::string formatTime(int seconds);

    std::string serialize() const;
    static Exercise deserialize(const std::string& data);
};

#endif // EXERCISE_H
