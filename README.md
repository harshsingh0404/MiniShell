# MiniShell
A custom Linux MiniShell implemented in C with support for built-in commands, external commands, pipes, signal handling (Ctrl+C, Ctrl+Z), and dynamic shell prompt.
# MiniShell

A Linux MiniShell implemented in C.

## Features

- Built-in commands
  - cd
  - pwd
  - exit
  - echo $$
  - echo $?
  - echo $SHELL

- External command execution
- Multiple pipes
- Ctrl+C handling
- Ctrl+Z handling
- Dynamic Linux-style prompt
- Colored prompt

## Technologies

- C Programming
- Linux System Calls
- fork()
- execvp()
- waitpid()
- pipe()
- signal()
