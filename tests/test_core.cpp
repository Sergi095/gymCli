#include "Exercise.h"
#include "GymDatabse.h"
#include "RoutineCsvImporter.h"
#include "Utils.h"
#include "WorkoutRoutine.h"
#include <cassert>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>

int main() {
    assert(isValidDate("2024-02-29"));
    assert(!isValidDate("2023-02-29"));
    assert(!isValidDate("Tuesday"));
    assert(getDayOfWeek("2024-02-29") == "Thursday");
    assert(getDayForDateOrLegacy("tue") == "Tuesday");
    assert(normalizeDayOfWeek(" FRI ") == "Friday");

    const std::string token = std::to_string(static_cast<long long>(std::time(nullptr)));
    const std::string dataFile = "gymcli_test_" + token + "_data.db";
    const std::string routinesFile = "gymcli_test_" + token + "_routines.db";
    const std::string validCsvFile = "gymcli_test_" + token + "_routine.csv";
    const std::string invalidCsvFile = "gymcli_test_" + token + "_invalid.csv";
    const std::string legacyDataFile = "gymcli_test_" + token + "_legacy_data.db";
    const std::string legacyRoutinesFile = "gymcli_test_" + token + "_legacy_routines.db";
    std::remove(dataFile.c_str());
    std::remove(routinesFile.c_str());
    std::remove(legacyDataFile.c_str());
    std::remove(legacyRoutinesFile.c_str());

    {
        GymDatabase database(dataFile, routinesFile);
        assert(database.getAllRoutines().empty());
        assert(database.getActiveRoutine() == nullptr);

        WorkoutRoutine defaultRoutine("Default Routine");
        database.addRoutine(defaultRoutine);

        Exercise bench("Bench Press", "Chest", MeasurementType::REPS, BodyPart::UPPER);
        bench.setDate("2026-08-11");
        bench.setRoutineName("Default Routine");
        bench.addRepSet(10, 50);
        assert(bench.getGoogleSearchUrl() ==
               "https://www.google.com/search?q=Bench+Press+exercise+technique");
        database.addExercise(bench);

        Exercise heavierBench("bench press", "Chest", MeasurementType::REPS, BodyPart::UPPER);
        heavierBench.setDate("2026-08-11");
        heavierBench.setRoutineName("Default Routine");
        heavierBench.addRepSet(8, 60);
        database.addExercise(heavierBench);

        Exercise legacySquat("Squat", "Legs", MeasurementType::REPS, BodyPart::LOWER);
        legacySquat.setDate("Monday");
        legacySquat.addRepSet(5, 80);
        database.addExercise(legacySquat);

        assert(database.getExercisesByDay("Tuesday").size() == 2);
        assert(database.getExercisesByDay("mon").size() == 1);
        assert(database.getExercisesByDay("not-a-day").empty());

        const auto progress = database.getProgressData("BENCH PRESS");
        assert(progress.size() == 1);
        assert(progress.at("2026-08-11") == 60);

        WorkoutRoutine renamedRoutine("Renamed Routine");
        assert(database.updateRoutine(0, renamedRoutine));
        assert(database.getExercisesByRoutine("Renamed Routine").size() == 2);
    }

    {
        WorkoutRoutine generatedDefault("Default Routine");
        generatedDefault.assignDayToBodyPart("Monday", BodyPart::UPPER);
        generatedDefault.assignDayToBodyPart("Tuesday", BodyPart::LOWER);
        generatedDefault.assignDayToBodyPart("Wednesday", BodyPart::FULL);
        generatedDefault.assignDayToBodyPart("Thursday", BodyPart::UPPER);
        generatedDefault.assignDayToBodyPart("Friday", BodyPart::LOWER);
        generatedDefault.assignDayToBodyPart("Saturday", BodyPart::FULL);
        generatedDefault.assignDayToBodyPart("Sunday", BodyPart::OTHER);
        std::ofstream routinesFileOutput(legacyRoutinesFile);
        routinesFileOutput << "0\n" << generatedDefault.serialize() << '\n';
    }
    {
        GymDatabase migratedDatabase(legacyDataFile, legacyRoutinesFile);
        assert(migratedDatabase.getAllRoutines().empty());
        assert(migratedDatabase.getActiveRoutine() == nullptr);
    }

    Exercise invalidBodyPart = Exercise::deserialize(
        "Run|Cardio|2026-08-11||TIME|99||60,0");
    assert(invalidBodyPart.getBodyPart() == BodyPart::OTHER);
    assert(invalidBodyPart.getDuration().size() == 1);

    WorkoutRoutine malformed = WorkoutRoutine::deserialize(
        "Safe routine|Monday=oops;Tuesday=1;");
    assert(malformed.getBodyPartForDay("Monday") == BodyPart::OTHER);
    assert(malformed.getBodyPartForDay("Tuesday") == BodyPart::LOWER);

    {
        std::ofstream csv(validCsvFile);
        csv << "routine,day,body_part,comment\n"
            << "\"Phone, Strength\",Mon,U,quoted comma works\n"
            << "\"Phone, Strength\",Tuesday,lower,\n"
            << "\"Phone, Strength\",Fri,rest,\n";
    }
    const RoutineCsvImportResult imported = RoutineCsvImporter::parseFile(validCsvFile);
    assert(imported.success);
    assert(imported.rowsRead == 3);
    assert(imported.routine.getName() == "Phone, Strength");
    assert(imported.routine.getBodyPartForDay("Monday") == BodyPart::UPPER);
    assert(imported.routine.getBodyPartForDay("Tuesday") == BodyPart::LOWER);
    assert(imported.routine.getBodyPartForDay("Wednesday") == BodyPart::OTHER);
    assert(imported.routine.getBodyPartForDay("Friday") == BodyPart::OTHER);

    {
        std::ofstream csv(invalidCsvFile);
        csv << "routine,day,body_part\n"
            << "Duplicate,Monday,upper\n"
            << "Duplicate,Mon,lower\n";
    }
    const RoutineCsvImportResult invalid = RoutineCsvImporter::parseFile(invalidCsvFile);
    assert(!invalid.success);
    assert(!invalid.errors.empty());

    const RoutineCsvImportResult fullBody =
        RoutineCsvImporter::parseFile("examples/muscle_strength.csv");
    assert(fullBody.success);
    assert(fullBody.routine.getName() == "Muscle Strength");
    assert(fullBody.routine.getFocus() == "Full Body");
    assert(fullBody.routine.getExercises().size() == 13);
    assert(fullBody.routine.getExercises()[0].name == "Static squat");
    assert(fullBody.routine.getExercises()[0].link.find("google.com/search") != std::string::npos);
    assert(fullBody.routine.getExercises()[11].section == "Flexibility");

    const WorkoutRoutine restored = WorkoutRoutine::deserialize(fullBody.routine.serialize());
    assert(restored.getName() == "Muscle Strength");
    assert(restored.getFocus() == "Full Body");
    assert(restored.getNotes() == fullBody.routine.getNotes());
    assert(restored.getExercises().size() == fullBody.routine.getExercises().size());
    assert(restored.getExercises()[5].notes == fullBody.routine.getExercises()[5].notes);

    std::remove(dataFile.c_str());
    std::remove(routinesFile.c_str());
    std::remove(validCsvFile.c_str());
    std::remove(invalidCsvFile.c_str());
    std::remove(legacyDataFile.c_str());
    std::remove(legacyRoutinesFile.c_str());
    std::cout << "All core tests passed.\n";
    return 0;
}
