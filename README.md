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

## Features

- **Exercise tracking**: Record reps or duration, weight, notes, and volume
- **Dated history**: Group workouts into sessions and filter them by weekday
- **Workout routines**: Assign upper, lower, full-body, or rest days
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
