#include "Exercise.h"
#include "GymDatabse.h"
#include "Utils.h"
#include "WorkoutRoutine.h"
#include <cassert>
#include <cstdio>
#include <ctime>
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
    std::remove(dataFile.c_str());
    std::remove(routinesFile.c_str());

    {
        GymDatabase database(dataFile, routinesFile);

        Exercise bench("Bench Press", "Chest", MeasurementType::REPS, BodyPart::UPPER);
        bench.setDate("2026-08-11");
        bench.setRoutineName("Default Routine");
        bench.addRepSet(10, 50);
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

    Exercise invalidBodyPart = Exercise::deserialize(
        "Run|Cardio|2026-08-11||TIME|99||60,0");
    assert(invalidBodyPart.getBodyPart() == BodyPart::OTHER);
    assert(invalidBodyPart.getDuration().size() == 1);

    WorkoutRoutine malformed = WorkoutRoutine::deserialize(
        "Safe routine|Monday=oops;Tuesday=1;");
    assert(malformed.getBodyPartForDay("Monday") == BodyPart::OTHER);
    assert(malformed.getBodyPartForDay("Tuesday") == BodyPart::LOWER);

    std::remove(dataFile.c_str());
    std::remove(routinesFile.c_str());
    std::cout << "All core tests passed.\n";
    return 0;
}
