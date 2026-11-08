#include "GymCliApp.h"
#include "RoutineCsvImporter.h"
#include "TableRenderer.h"
#include <iostream>
#include <string>

namespace {

void printUsage(const char* program) {
    std::cout << "Usage:\n"
              << "  " << program << "\n"
              << "  " << program << " --import-routine FILE.csv [--replace]\n"
              << "  " << program << " --preview-routine FILE.csv\n";
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc == 2 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")) {
        printUsage(argv[0]);
        return 0;
    }

    const bool importing = argc >= 3 && argc <= 4 &&
        std::string(argv[1]) == "--import-routine";
    const bool previewing = argc == 3 && std::string(argv[1]) == "--preview-routine";
    if (argc != 1 && !importing && !previewing) {
        printUsage(argv[0]);
        return 1;
    }

    if (previewing) {
        const RoutineCsvImportResult parsed = RoutineCsvImporter::parseFile(argv[2]);
        if (!parsed.success) {
            std::cout << "Routine CSV is invalid:\n";
            for (const auto& error : parsed.errors) {
                std::cout << "  - " << error << '\n';
            }
            return 1;
        }
        std::cout << "Valid routine CSV: " << parsed.rowsRead << " row(s), "
                  << parsed.routine.getExercises().size() << " exercise(s).\n";
        TableRenderer::renderRoutineChart(parsed.routine);
        return 0;
    }

    GymCliApp app;
    if (importing) {
        const bool replaceExisting = argc == 4 && std::string(argv[3]) == "--replace";
        if (argc == 4 && !replaceExisting) {
            printUsage(argv[0]);
            return 1;
        }
        return app.importRoutineCsvFile(argv[2], replaceExisting) ? 0 : 1;
    }

    app.run();
    return 0;
}
