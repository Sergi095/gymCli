#include "RoutineCsvImporter.h"
#include "Utils.h"
#include <fstream>
#include <map>
#include <sstream>

namespace {

enum class RecordStatus {
    RECORD,
    END,
    ERROR
};

RecordStatus readRecord(std::istream& input, std::vector<std::string>& fields,
                        std::string& error, size_t& lineNumber) {
    fields.clear();
    std::string field;
    bool inQuotes = false;
    bool quoteClosed = false;
    bool readAnything = false;
    char character = '\0';

    while (input.get(character)) {
        readAnything = true;

        if (inQuotes) {
            if (character == '"') {
                if (input.peek() == '"') {
                    input.get(character);
                    field += '"';
                } else {
                    inQuotes = false;
                    quoteClosed = true;
                }
            } else {
                field += character;
                if (character == '\n') {
                    ++lineNumber;
                }
            }
            continue;
        }

        if (quoteClosed) {
            if (character == ',') {
                fields.push_back(field);
                field.clear();
                quoteClosed = false;
            } else if (character == '\n' || character == '\r') {
                if (character == '\r' && input.peek() == '\n') {
                    input.get(character);
                }
                ++lineNumber;
                fields.push_back(field);
                return RecordStatus::RECORD;
            } else if (character != ' ' && character != '\t') {
                error = "Unexpected character after a quoted field.";
                return RecordStatus::ERROR;
            }
            continue;
        }

        if (character == ',') {
            fields.push_back(field);
            field.clear();
        } else if (character == '\n' || character == '\r') {
            if (character == '\r' && input.peek() == '\n') {
                input.get(character);
            }
            ++lineNumber;
            fields.push_back(field);
            return RecordStatus::RECORD;
        } else if (character == '"') {
            if (!field.empty()) {
                error = "A quote must start at the beginning of a field.";
                return RecordStatus::ERROR;
            }
            inQuotes = true;
        } else {
            field += character;
        }
    }

    if (inQuotes) {
        error = "Quoted field is not closed.";
        return RecordStatus::ERROR;
    }
    if (readAnything || !field.empty() || !fields.empty()) {
        fields.push_back(field);
        return RecordStatus::RECORD;
    }
    return RecordStatus::END;
}

bool isBlankRecord(const std::vector<std::string>& fields) {
    for (const auto& field : fields) {
        if (!trim(field).empty()) {
            return false;
        }
    }
    return true;
}

std::string normalizeHeader(std::string value) {
    value = trim(value);
    if (value.size() >= 3 &&
        static_cast<unsigned char>(value[0]) == 0xEF &&
        static_cast<unsigned char>(value[1]) == 0xBB &&
        static_cast<unsigned char>(value[2]) == 0xBF) {
        value.erase(0, 3);
    }

    value = toLower(value);
    for (auto& character : value) {
        if (character == ' ' || character == '-') {
            character = '_';
        }
    }
    return value;
}

int findColumn(const std::vector<std::string>& headers,
               const std::vector<std::string>& aliases) {
    for (size_t index = 0; index < headers.size(); ++index) {
        for (const auto& alias : aliases) {
            if (headers[index] == alias) {
                return static_cast<int>(index);
            }
        }
    }
    return -1;
}

std::string valueAt(const std::vector<std::string>& fields, int index) {
    if (index < 0 || static_cast<size_t>(index) >= fields.size()) {
        return "";
    }
    return trim(fields[static_cast<size_t>(index)]);
}

bool parseBodyPart(const std::string& value, BodyPart& bodyPart) {
    std::string normalized = toLower(trim(value));
    for (auto& character : normalized) {
        if (character == '_' || character == '-') {
            character = ' ';
        }
    }

    if (normalized == "u" || normalized == "upper" || normalized == "upper body") {
        bodyPart = BodyPart::UPPER;
        return true;
    }
    if (normalized == "l" || normalized == "lower" || normalized == "lower body") {
        bodyPart = BodyPart::LOWER;
        return true;
    }
    if (normalized == "f" || normalized == "full" || normalized == "full body") {
        bodyPart = BodyPart::FULL;
        return true;
    }
    if (normalized == "r" || normalized == "rest" || normalized == "rest day" ||
        normalized == "o" || normalized == "other" || normalized == "off" ||
        normalized == "none") {
        bodyPart = BodyPart::OTHER;
        return true;
    }
    return false;
}

std::string rowError(size_t line, const std::string& message) {
    return "Line " + std::to_string(line) + ": " + message;
}

} // namespace

RoutineCsvImportResult::RoutineCsvImportResult()
    : success(false), routine(""), rowsRead(0) {}

RoutineCsvImportResult RoutineCsvImporter::parseFile(const std::string& filename) {
    RoutineCsvImportResult result;
    std::ifstream input(filename);
    if (!input.is_open()) {
        result.errors.push_back("Could not open CSV file: " + filename);
        return result;
    }

    size_t lineNumber = 1;
    size_t recordStartLine = lineNumber;
    std::vector<std::string> fields;
    std::string parseError;
    RecordStatus status = readRecord(input, fields, parseError, lineNumber);
    while (status == RecordStatus::RECORD && isBlankRecord(fields)) {
        recordStartLine = lineNumber;
        status = readRecord(input, fields, parseError, lineNumber);
    }
    if (status == RecordStatus::ERROR) {
        result.errors.push_back(rowError(recordStartLine, parseError));
        return result;
    }
    if (status == RecordStatus::END) {
        result.errors.push_back("CSV file is empty.");
        return result;
    }

    std::vector<std::string> headers;
    for (const auto& field : fields) {
        headers.push_back(normalizeHeader(field));
    }

    const int routineColumn = findColumn(headers, {"routine", "routine_name", "name"});
    const int dayColumn = findColumn(headers, {"day", "weekday", "day_of_week"});
    const int bodyPartColumn = findColumn(headers, {"body_part", "bodypart", "workout"});
    const int focusColumn = findColumn(headers, {"focus", "routine_type", "target"});
    const int routineNotesColumn = findColumn(
        headers, {"routine_notes", "routine_instructions", "overview"});
    const int sectionColumn = findColumn(headers, {"section", "group"});
    const int exerciseColumn = findColumn(headers, {"exercise", "exercise_name", "movement"});
    const int setsColumn = findColumn(headers, {"sets", "set_count"});
    const int repsColumn = findColumn(headers, {"reps", "repetitions"});
    const int durationColumn = findColumn(headers, {"duration", "duration_seconds", "time"});
    const int weightColumn = findColumn(headers, {"weight", "weight_kg", "load"});
    const int restColumn = findColumn(headers, {"rest", "rest_between_sets"});
    const int exerciseNotesColumn = findColumn(headers, {"notes", "exercise_notes", "variations"});
    const int linkColumn = findColumn(headers, {"google_url", "google_link", "link", "url"});

    if (routineColumn < 0) {
        result.errors.push_back("Missing required 'routine' column.");
    }
    if ((dayColumn < 0) != (bodyPartColumn < 0)) {
        result.errors.push_back("The optional 'day' and 'body_part' columns must be used together.");
    }
    if (dayColumn < 0 && exerciseColumn < 0) {
        result.errors.push_back(
            "CSV needs either schedule columns ('day', 'body_part') or an 'exercise' column.");
    }
    if (!result.errors.empty()) {
        return result;
    }

    std::string routineName;
    std::string routineFocus;
    std::string routineNotes;
    std::map<std::string, size_t> assignedDays;
    size_t contentRows = 0;

    while (true) {
        recordStartLine = lineNumber;
        parseError.clear();
        status = readRecord(input, fields, parseError, lineNumber);
        if (status == RecordStatus::END) {
            break;
        }
        if (status == RecordStatus::ERROR) {
            result.errors.push_back(rowError(recordStartLine, parseError));
            break;
        }
        if (isBlankRecord(fields)) {
            continue;
        }

        ++result.rowsRead;
        if (fields.size() > headers.size()) {
            result.errors.push_back(rowError(recordStartLine, "Row has more fields than the header."));
            continue;
        }
        fields.resize(headers.size());

        const std::string rowRoutine = valueAt(fields, routineColumn);
        if (rowRoutine.empty()) {
            result.errors.push_back(rowError(recordStartLine, "Routine name is empty."));
            continue;
        }
        if (rowRoutine.find_first_of("|\r\n") != std::string::npos) {
            result.errors.push_back(rowError(
                recordStartLine, "Routine name contains a reserved character."));
            continue;
        }
        if (routineName.empty()) {
            routineName = rowRoutine;
            result.routine.setName(routineName);
        } else if (!equalsIgnoreCase(routineName, rowRoutine)) {
            result.errors.push_back(rowError(
                recordStartLine, "All rows must use the same routine name ('" + routineName + "')."));
            continue;
        }

        const std::string rowFocus = valueAt(fields, focusColumn);
        if (!rowFocus.empty()) {
            if (routineFocus.empty()) {
                routineFocus = rowFocus;
                result.routine.setFocus(rowFocus);
            } else if (!equalsIgnoreCase(routineFocus, rowFocus)) {
                result.errors.push_back(rowError(
                    recordStartLine, "All rows must use the same routine focus ('" +
                    routineFocus + "')."));
                continue;
            }
        }

        const std::string rowRoutineNotes = valueAt(fields, routineNotesColumn);
        if (!rowRoutineNotes.empty()) {
            if (routineNotes.empty()) {
                routineNotes = rowRoutineNotes;
                result.routine.setNotes(rowRoutineNotes);
            } else if (routineNotes != rowRoutineNotes) {
                result.errors.push_back(rowError(
                    recordStartLine, "Routine instructions differ from earlier rows."));
                continue;
            }
        }

        bool rowHasContent = false;
        const std::string rawDay = valueAt(fields, dayColumn);
        const std::string rawBodyPart = valueAt(fields, bodyPartColumn);
        if (!rawDay.empty() || !rawBodyPart.empty()) {
            if (rawDay.empty() || rawBodyPart.empty()) {
                result.errors.push_back(rowError(
                    recordStartLine, "Both day and body_part are required for a schedule row."));
                continue;
            }
            const std::string day = normalizeDayOfWeek(rawDay);
            BodyPart bodyPart = BodyPart::OTHER;
            if (day.empty()) {
                result.errors.push_back(rowError(recordStartLine, "Invalid weekday."));
                continue;
            }
            if (assignedDays.find(day) != assignedDays.end()) {
                result.errors.push_back(rowError(
                    recordStartLine, day + " was already assigned on line " +
                    std::to_string(assignedDays[day]) + "."));
                continue;
            }
            if (!parseBodyPart(rawBodyPart, bodyPart)) {
                result.errors.push_back(rowError(
                    recordStartLine, "Invalid body part; use upper, lower, full, or rest."));
                continue;
            }
            result.routine.assignDayToBodyPart(day, bodyPart);
            assignedDays[day] = recordStartLine;
            rowHasContent = true;
        }

        const std::string exerciseName = valueAt(fields, exerciseColumn);
        if (!exerciseName.empty()) {
            RoutineExercise exercise;
            exercise.section = valueAt(fields, sectionColumn);
            exercise.name = exerciseName;
            exercise.sets = valueAt(fields, setsColumn);
            exercise.reps = valueAt(fields, repsColumn);
            exercise.duration = valueAt(fields, durationColumn);
            exercise.weight = valueAt(fields, weightColumn);
            exercise.rest = valueAt(fields, restColumn);
            exercise.notes = valueAt(fields, exerciseNotesColumn);
            exercise.link = valueAt(fields, linkColumn);
            if (exercise.link.empty()) {
                exercise.link = makeGoogleSearchUrl(exercise.name);
            }
            result.routine.addExercise(exercise);
            rowHasContent = true;
        }

        if (!rowHasContent) {
            result.errors.push_back(rowError(
                recordStartLine, "Row needs an exercise or a day/body_part assignment."));
            continue;
        }
        ++contentRows;
    }

    if (result.rowsRead == 0) {
        result.errors.push_back("CSV has a header but no routine rows.");
    } else if (contentRows == 0) {
        result.errors.push_back("CSV does not contain any valid schedule or exercise rows.");
    }
    result.success = result.errors.empty() && !routineName.empty();
    return result;
}
