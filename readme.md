# PMan
A simple processes manager/shell prompt.
Only supports basic commands, and can start processes in the background. Does not
handle pipes or any remotely complex shell syntax.


## Build instructions
 - Calling make in the source directory will produce the 'pman' executable
 - ./pman in the same directory will then start PMan


## Commands
  - bg (args): starts a command in the background. The first argument to bg is the command/process that will be run.
    all arguments to bg after this will be passed to the started process. Commands that fail to run will show the message
    'Error: Invalid command "[command]" '

  - bglist: lists running child processes of PMan that have been started by bg.
    Each process is listed as [pid]: [exec] ([status]) with [pid] being the process pid, [exec] being the
    command used to start it, and [status] being one of ACTIVE or STOPPED. Active processes are coloured green,
    stopped processes yellow.

  - bgkill (pid): takes a process pid as it's only argument and kills said process. Only kills processes started direcly by
    PMan for safety, will print an error message otherwise.

  - bgstop (pid): similar to bgkill, however stops the processes with the given pid instead of killing it

  - bgstart (pid): resumes a process stopped by bgstop

  - pstat (pid): prints process information from /proc/(pid)/stat

  - quit and/or exit: either command will exit PMan, killing all background processes.

## Notes
PMan operates on a 1 second duration event loop, where after 1 second of waiting for input it
checks if any child processes have terminated before looping. If any have, PMan prints a message 
"Process (pid) has exited" for each terminated processes before the output of the entered command.
This will draw on top of user input, however it won't delete it.

Status messages are not printed for processes killed directly by bgkill, as bgkill prints it's own message.
