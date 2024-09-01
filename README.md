# MyShell - Command line interpreter

## Date - 2022

This project implements a command line interpreter called MyShell in the C programming language, designed to run in GNU/Linux environments. The main goal is to provide an interactive shell with basic functionalities and the ability to run internal commands, external programs, and scripts in the background.

### Authors:
- **Robledo, Valentín**

## Main Features

### 1. Command Line Prompt
MyShell presents a prompt indicating the username, hostname, and current directory. The format is as follows:

```
username@hostname:~$s
```

### 2. Internal Commands
MyShell supports several built-in commands:

- **cd \<directory\>**: Changes the current directory to \<directory\>. If \<directory\> is not present, the current directory is reported. If the directory does not exist, an error message is printed. Additionally, the command changes the PWD environment variable. It also supports the `cd -` option, which returns the last working directory (OLDPWD).

- **clr**: Clean the screen.

- **echo \<comment\|env var\>**: Displays \<comment\> on the screen followed by a new line. Multiple spaces/tabs can be reduced to one space.

- **quit**: Close MyShell.

### 3. Program Invocation
User input that is not built-in commands is interpreted as the invocation of a program. Execution is done via `fork` and `execl`. MyShell supports both relative and absolute paths.

### 4. Batch File
MyShell can take its commands from a file when invoked with an argument. For example:

```
./myshell batchfile
```

The batchfile contains a set of one-line commands for MyShell to execute. When the end of file (EOF) is reached, MyShell exits.

### 5. Background Execution
If a command ends with an ampersand (&), it indicates that the shell should return to the prompt immediately after launching the program in the background. A message is printed indicating the job and the process ID:

```
[<job id>] <process id>
```

Example:
```
$ echo 'hello' &
[1] 10506
hello
```
