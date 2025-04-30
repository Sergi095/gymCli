#ifndef TABLE_RENDERER_H
#define TABLE_RENDERER_H

#include <vector>
#include <map>
#include <string>
#include "Exercise.h"

class TableRenderer {
public:
    static void renderExercise(const Exercise& exercise);
    static void renderExerciseList(const std::vector<Exercise>& exercises);
    static void renderSessionList(const std::map<std::string, std::vector<Exercise>>& sessionMap);
    static void renderProgressChart(const std::string& exerciseName,
                                 const std::map<std::string, double>& progressData);
};

#endif // TABLE_RENDERER_H
