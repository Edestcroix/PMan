PMan

Build instructions
 - Calling make in the source directory will produce the 'pman' executable
 - ./pman in the same directory will then start PMan


Commands
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

PMan checks if any child processes have terminated whenever a command is entered. Pman prints the message 
"Process (pid) has exited" for each terminated processes before the output of the entered command.

Status messages are not printed for processes killed directly by bgkill, as bgkill prints it's own message.
