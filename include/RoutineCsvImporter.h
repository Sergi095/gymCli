#ifndef ROUTINE_CSV_IMPORTER_H
#define ROUTINE_CSV_IMPORTER_H

#include "WorkoutRoutine.h"
#include <cstddef>
#include <string>
#include <vector>

struct RoutineCsvImportResult {
    bool success;
    WorkoutRoutine routine;
    size_t rowsRead;
    std::vector<std::string> errors;

    RoutineCsvImportResult();
};

class RoutineCsvImporter {
public:
    static RoutineCsvImportResult parseFile(const std::string& filename);
};

#endif // ROUTINE_CSV_IMPORTER_H
