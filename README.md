# HKU-COMP3230-Shell

## Introduction
This project simulates a Linux terminal shell utilizing system programming concepts like process creation (`fork()`), inter-process communication (`pipe()`), and signal handling. It was developed as part of the COMP3230 Principles of Operating Systems course at The University of Hong Kong.

### Achievements
- Scored 11.5/12 for core functionality.
- Scored 4/4 for bonus features.

## Features
- Executes commands by creating child processes.
- Handles absolute, relative, and `PATH` environment variable paths for command execution.
- Supports built-in commands:
  - `exit`: Terminates the shell.
  - `timeX`: Prints process statistics of terminated child processes.
- Implements operators:
  - `&`: Executes commands in the background.
  - `|`: Pipes the output of one command as the input to another.
- Robust to `SIGINT` (Ctrl-C) interruptions.
- Handles `SIGUSR1` for controlled execution of child processes.
- Handles `SIGCHLD` for background process termination.

## Quick Start

1. Clone the repository:
   ```bash
   git clone https://github.com/Henryyy-Hung/HKU-COMP3230-Shell
   ```
2. Navigate to the project directory:
   ```bash
   cd HKU-COMP3230-Shell/src
   ```
3. Build the project:
   ```bash
   make all
   ```
4. Run the shell:
   ```bash
   ./3230shell
   ```

## Preview
Here's a snapshot of what the shell looks like in action:
![Shell Preview](https://user-images.githubusercontent.com/78750074/208289917-8b969d99-2be8-4bfd-b2d6-9211568459f2.png)
