#include "TableRenderer.h"
#include "Utils.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <sstream>

namespace {

std::string shorten(const std::string& value, size_t width) {
    if (value.size() <= width) {
        return value;
    }
    if (width <= 3) {
        return value.substr(0, width);
    }
    return value.substr(0, width - 3) + "...";
}

std::string number(double value) {
    std::ostringstream output;
    output << std::fixed << std::setprecision(1) << value;
    return output.str();
}

std::string dayLabel(const std::string& date) {
    const std::string day = getDayForDateOrLegacy(date);
    return day == "Unknown" ? "" : day;
}

std::string dateAndDay(const std::string& date) {
    const std::string day = dayLabel(date);
    if (isValidDate(date) && !day.empty()) {
        return date + " (" + day + ")";
    }
    if (!day.empty()) {
        return day + " (legacy record)";
    }
    return date.empty() ? "Date unavailable" : date;
}

std::string plural(size_t count, const std::string& singular, const std::string& multiple) {
    return std::to_string(count) + " " + (count == 1 ? singular : multiple);
}

std::string joinKeys(const std::map<std::string, bool>& values) {
    std::string result;
    for (const auto& value : values) {
        if (!result.empty()) {
            result += ", ";
        }
        result += value.first;
    }
    return result;
}

void rule(char character, int width) {
    std::cout << std::string(std::max(1, width), character) << '\n';
}

} // namespace

void TableRenderer::renderExercise(const Exercise& exercise) {
    const int width = std::min(getTerminalWidth(), 76);
    std::cout << '\n';
    rule('=', width);
    std::cout << shorten(exercise.getName(), width) << '\n';
    std::cout << shorten(exercise.getCategory() + " | " + exercise.getBodyPartString(), width) << '\n';
    std::cout << shorten(dateAndDay(exercise.getDate()), width) << '\n';
    if (!exercise.getRoutineName().empty()) {
        std::cout << shorten("Routine: " + exercise.getRoutineName(), width) << '\n';
    }
    rule('-', width);

    const bool repBased = exercise.getMeasurementType() == MeasurementType::REPS;
    std::cout << std::left
              << std::setw(6) << "Set"
              << std::setw(10) << (repBased ? "Reps" : "Time")
              << std::setw(13) << "Weight (kg)"
              << "Volume" << '\n';
    rule('-', std::min(width, 42));

    const auto& sets = exercise.getSets();
    const auto& reps = exercise.getReps();
    const auto& duration = exercise.getDuration();
    const auto& weights = exercise.getWeights();

    for (size_t i = 0; i < sets.size(); ++i) {
        const double setVolume = repBased
            ? reps[i] * weights[i]
            : (duration[i] / 60.0) * weights[i];
        const std::string amount = repBased
            ? std::to_string(reps[i])
            : Exercise::formatTime(duration[i]);

        std::cout << std::left
                  << std::setw(6) << sets[i]
                  << std::setw(10) << amount
                  << std::setw(13) << number(weights[i])
                  << number(setVolume) << '\n';
    }

    rule('-', std::min(width, 42));
    std::cout << "Total volume: " << number(exercise.calculateVolume()) << '\n';
    if (!exercise.getNotes().empty()) {
        std::cout << "Notes: " << shorten(exercise.getNotes(), width - 7) << '\n';
    }
    rule('=', width);
}

void TableRenderer::renderExerciseList(const std::vector<Exercise>& exercises) {
    if (exercises.empty()) {
        std::cout << "No exercises found.\n";
        return;
    }

    const int terminalWidth = getTerminalWidth();
    std::cout << '\n' << plural(exercises.size(), "exercise", "exercises") << '\n';

    if (terminalWidth < 105) {
        rule('-', terminalWidth);
        for (size_t i = 0; i < exercises.size(); ++i) {
            const Exercise& exercise = exercises[i];
            std::cout << shorten("[" + std::to_string(i + 1) + "] " + exercise.getName(), terminalWidth) << '\n';
            std::cout << "    " << shorten(dateAndDay(exercise.getDate()), terminalWidth - 4) << '\n';
            std::cout << "    " << shorten(exercise.getCategory() + " | " + exercise.getBodyPartString(), terminalWidth - 4) << '\n';

            std::string summary = plural(exercise.getSets().size(), "set", "sets") +
                                  " | volume " + number(exercise.calculateVolume());
            if (!exercise.getRoutineName().empty()) {
                summary += " | " + exercise.getRoutineName();
            }
            std::cout << "    " << shorten(summary, terminalWidth - 4) << '\n';
            if (i + 1 < exercises.size()) {
                std::cout << '\n';
            }
        }
        rule('-', terminalWidth);
        return;
    }

    const int width = 103;
    rule('=', width);
    std::cout << std::left
              << std::setw(4) << "ID"
              << std::setw(19) << "Exercise"
              << std::setw(14) << "Category"
              << std::setw(12) << "Date"
              << std::setw(11) << "Day"
              << std::setw(13) << "Body part"
              << std::setw(14) << "Routine"
              << std::setw(6) << "Sets"
              << "Volume" << '\n';
    rule('-', width);

    for (size_t i = 0; i < exercises.size(); ++i) {
        const Exercise& exercise = exercises[i];
        std::cout << std::left
                  << std::setw(4) << (i + 1)
                  << std::setw(19) << shorten(exercise.getName(), 18)
                  << std::setw(14) << shorten(exercise.getCategory(), 13)
                  << std::setw(12) << shorten(exercise.getDate(), 11)
                  << std::setw(11) << shorten(dayLabel(exercise.getDate()), 10)
                  << std::setw(13) << shorten(exercise.getBodyPartString(), 12)
                  << std::setw(14) << shorten(exercise.getRoutineName(), 13)
                  << std::setw(6) << exercise.getSets().size()
                  << number(exercise.calculateVolume()) << '\n';
    }
    rule('=', width);
}

void TableRenderer::renderSessionList(
    const std::map<std::string, std::vector<Exercise>>& sessionMap) {
    if (sessionMap.empty()) {
        std::cout << "No workout sessions found.\n";
        return;
    }

    const int terminalWidth = getTerminalWidth();
    std::cout << '\n' << plural(sessionMap.size(), "session", "sessions") << '\n';

    size_t sessionNumber = 1;
    for (const auto& session : sessionMap) {
        std::map<std::string, bool> categories;
        std::map<std::string, bool> bodyParts;
        std::map<std::string, bool> routines;
        size_t totalSets = 0;
        double totalVolume = 0;

        for (const auto& exercise : session.second) {
            categories[exercise.getCategory()] = true;
            bodyParts[exercise.getBodyPartString()] = true;
            if (!exercise.getRoutineName().empty()) {
                routines[exercise.getRoutineName()] = true;
            }
            totalSets += exercise.getSets().size();
            totalVolume += exercise.calculateVolume();
        }

        rule('-', terminalWidth);
        std::cout << shorten("[" + std::to_string(sessionNumber) + "] " +
                             dateAndDay(session.first), terminalWidth) << '\n';
        std::cout << shorten(plural(session.second.size(), "exercise", "exercises") +
                             " | " + plural(totalSets, "set", "sets") +
                             " | volume " + number(totalVolume), terminalWidth) << '\n';
        std::cout << shorten("Body: " + joinKeys(bodyParts), terminalWidth) << '\n';
        if (!routines.empty()) {
            std::cout << shorten("Routine: " + joinKeys(routines), terminalWidth) << '\n';
        }
        std::cout << shorten("Categories: " + joinKeys(categories), terminalWidth) << '\n';
        ++sessionNumber;
    }
    rule('-', terminalWidth);
}

void TableRenderer::renderProgressChart(
    const std::string& exerciseName,
    const std::map<std::string, double>& progressData) {
    if (progressData.empty()) {
        std::cout << "No progress data available for " << exerciseName << ".\n";
        return;
    }

    const int width = std::min(getTerminalWidth(), 80);
    const int chartWidth = std::max(10, width - 28);
    double maxValue = 0;
    for (const auto& entry : progressData) {
        maxValue = std::max(maxValue, entry.second);
    }

    std::cout << '\n' << shorten("Progress: " + exerciseName, width) << '\n';
    rule('-', width);
    for (const auto& entry : progressData) {
        const int barLength = maxValue > 0
            ? static_cast<int>((entry.second / maxValue) * chartWidth)
            : 0;
        std::cout << std::left << std::setw(12) << shorten(entry.first, 11)
                  << " | " << std::string(barLength, '#')
                  << (barLength > 0 ? " " : "") << number(entry.second) << " kg\n";
    }
    rule('-', width);
}
