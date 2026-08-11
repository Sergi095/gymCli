# GymCLI

A small, dependency-free workout tracker for the terminal.

## What it does

GymCLI records exercises with sets, reps or duration, weight, notes, and the
actual workout date. It can filter history by exercise, category, weekday,
body part, or routine, and includes session summaries and progress charts.

The interface adapts to narrow terminals, including mobile terminal apps:

```text
==========================================
| GYMCLI                                 |
| 2026-08-11 - Tuesday                   |
| Today: Lower Body                      |
| Routine: Default Routine               |
==========================================

Workout
  [1] Log an exercise
  [2] Exercise history
  [8] Sessions by date
  [9] Exercise progress
```

## Installation

### Build from source

```bash
# Clone the repository
git clone https://github.com/Sergi095/gymCli.git
cd gymCli

# Build
make

# Install
make install

# Run
gymcli
```

### Uninstall

```bash
make uninstall
```

Your workout data is kept in place when the program is uninstalled.

### Termux on Android

GymCLI detects Termux automatically. It uses `clang++`, installs into the
writable Termux prefix, and does not call `sudo`.

```bash
pkg update
pkg install git clang make
git clone https://github.com/Sergi095/gymCli.git
cd gymCli
make
make install
gymcli
```

### Run without installing

```bash
make run
```

### Run the checks

```bash
make test
```

## Import a routine from CSV

Import from the routine menu, or pass a file directly:

```bash
gymcli --import-routine examples/muscle_strength.csv
gymcli --import-routine examples/push_pull_legs.csv
```

Use `--replace` to update a routine that already has the same name:

```bash
gymcli --import-routine routine.csv --replace
```

Every row must contain `routine`. Exercise routines use `exercise` and may
also include `focus`, `routine_notes`, `section`, `sets`, `reps`, `duration`,
`weight`, `rest`, `notes`, and `google_url`. When `google_url` is blank,
GymCLI creates a Google exercise-technique search link automatically.

A CSV can optionally schedule the routine with paired `day` and `body_part`
columns. Accepted body parts are `upper`, `lower`, `full`, and `rest`; short
weekday and body-part codes such as `Mon,U` also work. Unspecified days remain
rest days. See [`examples/muscle_strength.csv`](examples/muscle_strength.csv)
for the converted full-body routine and
[`examples/push_pull_legs.csv`](examples/push_pull_legs.csv) for a four-section
push/pull/legs routine with stretches.

## Features

- **Exercise tracking**: Record reps or duration, weight, notes, and volume
- **Dated history**: Group workouts into sessions and filter them by weekday
- **Workout routines**: Assign upper, lower, full-body, or rest days
- **CSV routine import**: Load exercises, instructions, prescriptions, and schedules
- **Exercise links**: Show clickable Google technique searches in exercise details
- **Progress charts**: Compare the maximum weight by date
- **Responsive output**: Compact cards on narrow screens and tables on wide ones
- **Validated prompts**: Bad input is explained and requested again instead of being silently changed

## Technical stuff

- Written in C++11 with no external dependencies
- Uses local text files for storage
- Source runs store data in the current directory
- Installed runs store data under `~/.local/share/gymcli` on Linux, macOS, and Termux

## Other

This is just a personal project. Feel free to fork it if you want to modify it for your own use.
