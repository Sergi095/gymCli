#!/bin/sh
set -eu

binary=$(cd "$(dirname "$1")" && pwd)/$(basename "$1")
test_dir=$(mktemp -d)
trap 'rm -rf "$test_dir"' EXIT HUP INT TERM
export GYMCLI_DATA_DIR="$test_dir"

cat > "$test_dir/routine.csv" <<'CSV'
routine,day,body_part,section,exercise,sets,reps
Test Routine,Mon,upper,Push,Bench Press,3,8
Test Routine,Monday,U,Push,Shoulder Press,3,10
Test Routine,Wed,lower,Legs,Squat,4,6
CSV

"$binary" --preview-routine "$test_dir/routine.csv" > "$test_dir/preview.txt"
grep -q 'Valid routine CSV: 3 row(s), 3 exercise(s)' "$test_dir/preview.txt"
grep -q 'Routine graph: Test Routine' "$test_dir/preview.txt"
grep -q 'Exercises by section' "$test_dir/preview.txt"
test ! -e "$test_dir/gym_routines.db"

"$binary" --import-routine "$test_dir/routine.csv" > "$test_dir/import.txt"
grep -q "Imported routine 'Test Routine'" "$test_dir/import.txt"
test -s "$test_dir/gym_routines.db"
if "$binary" --import-routine "$test_dir/routine.csv" > "$test_dir/duplicate.txt"; then
    echo 'Duplicate import unexpectedly succeeded' >&2
    exit 1
fi
"$binary" --import-routine "$test_dir/routine.csv" --replace > "$test_dir/replace.txt"
grep -q "Replaced routine 'Test Routine'" "$test_dir/replace.txt"

"$binary" > "$test_dir/workout.txt" <<'INPUT'
1
1






8
20
8
20
8
20

0
INPUT
grep -q 'Exercise: Bench Press' "$test_dir/workout.txt"
grep -q 'Workout saved.' "$test_dir/workout.txt"

"$binary" > "$test_dir/progress.txt" <<'INPUT'
9
1
0
INPUT
grep -q 'Progress: Bench Press' "$test_dir/progress.txt"
grep -q '# 20.0 kg' "$test_dir/progress.txt"

"$binary" > "$test_dir/cancel.txt" <<'INPUT'
1
/back
0
INPUT
grep -q 'Back to main menu.' "$test_dir/cancel.txt"
test "$(wc -l < "$test_dir/gym_data.db")" -eq 1

"$binary" > "$test_dir/second_workout.txt" <<'INPUT'
1
2


2030-01-01



10
12
10
12
10
12

0
INPUT
grep -q 'Exercise: Shoulder Press' "$test_dir/second_workout.txt"
test "$(wc -l < "$test_dir/gym_data.db")" -eq 2

"$binary" > "$test_dir/browse_exercises.txt" <<'INPUT'
2
1
n
n
p
l
2
b
0
INPUT
grep -q 'Shoulder Press' "$test_dir/browse_exercises.txt"
grep -q '\[n\]ext / \[p\]revious' "$test_dir/browse_exercises.txt"
grep -q 'Already at the last item.' "$test_dir/browse_exercises.txt"

"$binary" > "$test_dir/browse_progress.txt" <<'INPUT'
9
1
n
n
p
b
0
INPUT
grep -q 'Progress: Bench Press' "$test_dir/browse_progress.txt"
grep -q 'Progress: Shoulder Press' "$test_dir/browse_progress.txt"

"$binary" > "$test_dir/browse_sessions.txt" <<'INPUT'
8
1

n

p

b
0
INPUT
grep -q '2030-01-01' "$test_dir/browse_sessions.txt"
grep -q '\[n\]ext / \[p\]revious' "$test_dir/browse_sessions.txt"

cat > "$test_dir/second.csv" <<'CSV'
routine,day,body_part,exercise
Second Routine,Tuesday,lower,Squat
CSV
"$binary" --import-routine "$test_dir/second.csv" > "$test_dir/second_import.txt"
"$binary" > "$test_dir/browse_routines.txt" <<'INPUT'
10
1
1
n
n
p
b
0
0
INPUT
grep -q 'Routine graph: Test Routine' "$test_dir/browse_routines.txt"
grep -q 'Routine graph: Second Routine' "$test_dir/browse_routines.txt"
grep -q 'Already at the last item.' "$test_dir/browse_routines.txt"
test "$(grep -c 'Routine graph: Second Routine' "$test_dir/browse_routines.txt")" -eq 1

before=$(cksum "$test_dir/gym_routines.db")
"$binary" > "$test_dir/cancel_routine.txt" <<'INPUT'
10
2
Temporary Routine
U
/back
0
0
INPUT
grep -q 'Back to routines.' "$test_dir/cancel_routine.txt"
test "$(cksum "$test_dir/gym_routines.db")" = "$before"

cat > "$test_dir/invalid.csv" <<'CSV'
routine,day,body_part,exercise
Invalid,Mon,upper,Bench Press
Invalid,Mon,lower,Squat
CSV
if "$binary" --preview-routine "$test_dir/invalid.csv" > "$test_dir/invalid.txt"; then
    echo 'Conflicting schedule unexpectedly passed validation' >&2
    exit 1
fi
grep -q 'conflicts with the assignment on line 2' "$test_dir/invalid.txt"

echo 'CLI CSV and graph tests passed.'
