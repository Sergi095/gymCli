#include "Exercise.h"
#include "Utils.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

Exercise::Exercise(const std::string& n, const std::string& cat, MeasurementType type, BodyPart bp)
    : name(n), category(cat), date(getCurrentDate()), measurementType(type), bodyPart(bp), routineName("") {}

void Exercise::addRepSet(int setReps, double weight) {
    sets.push_back(sets.size() + 1); // Set number
    reps.push_back(setReps);
    duration.push_back(0); // Not used for rep-based exercises
    weights.push_back(weight);
}

void Exercise::addTimeSet(int setSeconds, double weight) {
    sets.push_back(sets.size() + 1); // Set number
    duration.push_back(setSeconds);
    reps.push_back(0); // Not used for time-based exercises
    weights.push_back(weight);
}

void Exercise::setNotes(const std::string& n) {
    notes = n;
}

void Exercise::setBodyPart(BodyPart bp) {
    bodyPart = bp;
}

void Exercise::setRoutineName(const std::string& name) {
    routineName = name;
}

void Exercise::setDate(const std::string& d) {
    date = d;
}

std::string Exercise::getName() const {
    return name;
}

std::string Exercise::getCategory() const {
    return category;
}

std::string Exercise::getDate() const {
    return date;
}

std::string Exercise::getNotes() const {
    return notes;
}

MeasurementType Exercise::getMeasurementType() const {
    return measurementType;
}

BodyPart Exercise::getBodyPart() const {
    return bodyPart;
}

std::string Exercise::getBodyPartString() const {
    switch (bodyPart) {
        case BodyPart::UPPER: return "Upper Body";
        case BodyPart::LOWER: return "Lower Body";
        case BodyPart::FULL: return "Full Body";
        default: return "Other";
    }
}

std::string Exercise::getRoutineName() const {
    return routineName;
}

const std::vector<int>& Exercise::getSets() const {
    return sets;
}

const std::vector<int>& Exercise::getReps() const {
    return reps;
}

const std::vector<int>& Exercise::getDuration() const {
    return duration;
}

const std::vector<double>& Exercise::getWeights() const {
    return weights;
}

double Exercise::calculateVolume() const {
    double volume = 0;

    if (measurementType == MeasurementType::REPS) {
        // For rep-based exercises: volume = weight × reps
        for (size_t i = 0; i < sets.size(); i++) {
            volume += weights[i] * reps[i];
        }
    } else {
        // For time-based exercises: volume = weight × duration (in minutes)
        for (size_t i = 0; i < sets.size(); i++) {
            // Convert seconds to minutes for calculation
            volume += weights[i] * (duration[i] / 60.0);
        }
    }

    return volume;
}

std::string Exercise::formatTime(int seconds) {
    std::stringstream ss;
    int minutes = seconds / 60;
    int remainingSeconds = seconds % 60;

    ss << minutes << ":" << std::setfill('0') << std::setw(2) << remainingSeconds;
    return ss.str();
}

std::string Exercise::serialize() const {
    std::string result = name + "|" + category + "|" + date + "|" + notes + "|";

    // Add measurement type
    result += (measurementType == MeasurementType::REPS ? "REPS|" : "TIME|");

    // Add body part
    result += std::to_string(static_cast<int>(bodyPart)) + "|";

    // Add routine name
    result += routineName + "|";

    // Add sets data
    for (size_t i = 0; i < sets.size(); i++) {
        if (measurementType == MeasurementType::REPS) {
            result += std::to_string(reps[i]);
        } else {
            result += std::to_string(duration[i]);
        }

        result += "," + std::to_string(weights[i]);

        if (i < sets.size() - 1) {
            result += ";";
        }
    }

    return result;
}




Exercise Exercise::deserialize(const std::string& data) {
    size_t pos = 0;
    size_t nextPos = data.find("|", pos);

    if (nextPos == std::string::npos) return Exercise();

    std::string name = data.substr(pos, nextPos - pos);

    pos = nextPos + 1;
    nextPos = data.find("|", pos);
    if (nextPos == std::string::npos) return Exercise(name);

    std::string category = data.substr(pos, nextPos - pos);

    pos = nextPos + 1;
    nextPos = data.find("|", pos);
    if (nextPos == std::string::npos) return Exercise(name, category);

    std::string date = data.substr(pos, nextPos - pos);

    pos = nextPos + 1;
    nextPos = data.find("|", pos);
    if (nextPos == std::string::npos) {
        Exercise ex(name, category);
        ex.setDate(date);
        return ex;
    }

    std::string notes = data.substr(pos, nextPos - pos);

    pos = nextPos + 1;
    nextPos = data.find("|", pos);
    if (nextPos == std::string::npos) {
        Exercise ex(name, category);
        ex.setDate(date);
        ex.setNotes(notes);
        return ex;
    }

    std::string typeStr = data.substr(pos, nextPos - pos);
    MeasurementType type = (typeStr == "REPS") ? MeasurementType::REPS : MeasurementType::TIME;

    pos = nextPos + 1;
    nextPos = data.find("|", pos);
    if (nextPos == std::string::npos) {
        Exercise ex(name, category, type);
        ex.setDate(date);
        ex.setNotes(notes);
        return ex;
    }

    // Parse body part (supports string and numeric)
    std::string bpStr = data.substr(pos, nextPos - pos);
    BodyPart bodyPart;
    if (bpStr == "UPPER") bodyPart = BodyPart::UPPER;
    else if (bpStr == "LOWER") bodyPart = BodyPart::LOWER;
    else if (bpStr == "FULL") bodyPart = BodyPart::FULL;
    else {
        try {
            int bodyPartInt = std::stoi(bpStr);
            if (bodyPartInt < static_cast<int>(BodyPart::UPPER) ||
                bodyPartInt > static_cast<int>(BodyPart::OTHER)) {
                bodyPart = BodyPart::OTHER;
            } else {
                bodyPart = static_cast<BodyPart>(bodyPartInt);
            }
        } catch (...) {
            bodyPart = BodyPart::OTHER;
        }
    }

    pos = nextPos + 1;
    nextPos = data.find("|", pos);
    if (nextPos == std::string::npos) {
        Exercise ex(name, category, type, bodyPart);
        ex.setDate(date);
        ex.setNotes(notes);
        return ex;
    }

    std::string routineName = data.substr(pos, nextPos - pos);

    Exercise ex(name, category, type, bodyPart);
    ex.setDate(date);
    ex.setNotes(notes);
    ex.setRoutineName(routineName);

    pos = nextPos + 1;
    std::string setsData = data.substr(pos);

    size_t setPos = 0;
    size_t nextSetPos;

    while ((nextSetPos = setsData.find(";", setPos)) != std::string::npos) {
        std::string setData = setsData.substr(setPos, nextSetPos - setPos);
        size_t commaPos = setData.find(",");
        if (commaPos != std::string::npos) {
            try {
                int val = std::stoi(setData.substr(0, commaPos));
                double weight = std::stod(setData.substr(commaPos + 1));
                if (type == MeasurementType::REPS)
                    ex.addRepSet(val, weight);
                else
                    ex.addTimeSet(val, weight);
            } catch (...) {
                // skip invalid set
            }
        }
        setPos = nextSetPos + 1;
    }

    if (setPos < setsData.length()) {
        std::string setData = setsData.substr(setPos);
        size_t commaPos = setData.find(",");
        if (commaPos != std::string::npos) {
            try {
                int val = std::stoi(setData.substr(0, commaPos));
                double weight = std::stod(setData.substr(commaPos + 1));
                if (type == MeasurementType::REPS)
                    ex.addRepSet(val, weight);
                else
                    ex.addTimeSet(val, weight);
            } catch (...) {
                // skip invalid last set
            }
        }
    }

    return ex;
}
