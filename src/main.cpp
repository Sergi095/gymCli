#include "GymCliApp.h"
#include <iostream>
#include <string>

namespace {

void printUsage(const char* program) {
    std::cout << "Usage:\n"
              << "  " << program << "\n"
              << "  " << program << " --import-routine FILE.csv [--replace]\n";
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc == 2 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")) {
        printUsage(argv[0]);
        return 0;
    }

    if (argc != 1 &&
        !(argc >= 3 && argc <= 4 && std::string(argv[1]) == "--import-routine")) {
        printUsage(argv[0]);
        return 1;
    }

    GymCliApp app;
    if (argc >= 3) {
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
