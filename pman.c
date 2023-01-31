#include "list.h"
#include "process.h"
#include "utils.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int MAX_BUFF_LEN = 80;//max amount of bytes to read from stdin
int MAX_ARGS = 100;   // max amount of unique arguments to read
int WAIT_TIME = 1;    // how often select should check for input, in seconds
int CMD = 0;          // position of the command to handle by PMan in the args list
int FIRST_ARG = 1;    // position of the first argument in the args list

/* function parse_cmds
 * -------------------
 * parses the input string into an array of commands/arguments
 * assumes input string uses spaces to separate commands/arguments
 * inputs: input - the input string
 *         args - the array of arguments
 */
void parse_cmds(char *input, char *args[]) {
  char *token = strtok(input, " ");
  int i = 0;
  while (token != NULL) {
    args[i] = token;
    i++;
    token = strtok(NULL, " ");
  }
  args[i] = NULL;
}

/* function: pid_from_args
 * -----------------------
 * wrapper function to handle common code for all command
 * handlers in handle_cmds that require getting a pid from the
 * arguments list.
 * inputs: args - the arguments passed to PMan
 * returns: the pid if it is valid, -1 otherwise.
 */
int pid_from_args(char *args[]) {
  if (args[FIRST_ARG] == NULL) {
    printf("Error: Expected argument\n");
    return -1;
  } else if (args[FIRST_ARG + 1] != NULL) {
    printf("Error: Too many arguments\n");
    return -1;
  }
  int pid = atoi(args[FIRST_ARG]);
  if (!pid) {
    printf("Error: Invalid argument \"%s\", expected process id\n",
           args[FIRST_ARG]);
    return -1;
  }
  return pid;
}

/* function: handle_cmds
 * ---------------------
 * handles the commands that PMan can execute, and calls the appropriate
 * functions to handle them.
 * inputs: args - the arguments passed to PMan
 *         processes - the list of background processes
 * returns: 1 if the command was handled,
 *          -1 if the command was quit or exit
 */
int handle_cmds(char *args[], proc_list_t *processes) {
  char *cmd = args[CMD];
  if (strcmp(cmd, "bg") == 0) {
    // remove the "bg" command from args. Everything after are
    // the command/arguments for the child process to execute.
    if (args[1] == NULL) {
      printf("Error: Expected arguments\n");
    } else {
      remove_first(args);
      fork_process(args, processes);
    }

  } else if (strcmp(cmd, "bglist") == 0) {
    args[FIRST_ARG] != NULL ? printf("Error: Unexpected argument(s)\n")
                            : list_processes(processes);

  } else if (strcmp(cmd, "bgkill") == 0) {
    // if the pid is invalid, pid_from_args will print an error message,
    // and return -1. send_signal just returns immediately in this case.
    send_signal(processes, pid_from_args(args), SIGKILL);

  } else if (strcmp(cmd, "bgstop") == 0) {
    send_signal(processes, pid_from_args(args), SIGSTOP);

  } else if (strcmp(cmd, "bgstart") == 0) {
    send_signal(processes, pid_from_args(args), SIGCONT);

  } else if (strcmp(cmd, "pstat") == 0) {
    int pid = pid_from_args(args);
    if (pid != -1)
      print_pstats(pid);

  } else if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "exit") == 0) {
    return -1;

  } else {
    printf("Error: Invalid command\n");
  }
  return 1;
}

/* function: check_input
 * uses select() to determine if input can be read from stdin
 * returns 1 if input can be read, 0 otherwise
 */
int check_input() {
  struct timeval tv = {.tv_sec = WAIT_TIME, .tv_usec = 0};
  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(fileno(stdin), &readfds);
  return select(fileno(stdin) + 1, &readfds, NULL, NULL, &tv);
}

/* function: handle_input
 * checks for user input when called. If input is present,
 * process the input and parses the provided commands, executing
 * them if they are valid. Otherwise check if any child processes
 * have terminated and display a message if they have.
 * inputs: - args: array to hold the parse command arguments
 *         - processes: list of managed processes
 * output: - -1 if input is quit or exit
 *         - 0 if no input found
 *         - 1 if input is found
 *         - terminates with an error if the select() call fails
 */
int handle_input(proc_list_t *processes) {
  char input[MAX_BUFF_LEN];
  char *args[MAX_ARGS];
  char raw_input[MAX_BUFF_LEN];
  int child_ended = 0;

  switch (check_input()) {
  case -1:
    perror("select");
    exit(1);
  case 0:
    return check_processes(processes);
  default:
    // read from stdin
    read(fileno(stdin), raw_input, MAX_BUFF_LEN);
    // parse the raw buffer, prevents segfaults.
    sscanf(raw_input, "%[^\t\n]", input);

    // don't parse input if it is only space chars.
    if (!all_spaces(input)) {
      remove_newline(input);
      parse_cmds(input, args);
      return handle_cmds(args, processes);
    } else {
      printf("Error: Invalid command\n");
    }
    return 1;
  }
}

/* function: main
 * --------------
 * main function for PMan. Handles the main loop of the program,
 * and calls the appropriate functions to handle commands.
 * waits for input using select, and checks for background processes
 * that have exited while waiting. On input it reads the input, parses it,
 * and calls handle_cmds to handle the command.
 */
int main() {
  int quit = 0;
  int need_prompt = 1;
  proc_list_t *processes = create_list();
  char *args[MAX_ARGS];

  // main event loop
  while (!quit) {
    // print prompt only if needed, preventing printing multiple prompts
    // every loop iteration and allowing finer control over displaying prompt
    if (need_prompt) {
      printf("PMan: > ");
      need_prompt = 0;
    }

    fflush(stdout);

    // check for user input, then if no input is provided, check if
    // processes have terminated. Signal prompt to be redrawn
    // whenever input is provided or a process termination message is sent
    int is_input = handle_input(processes);
    // check_processes returns 1 when it prints a termination message,
    // so setting need_prompt to check_processes output will reprint
    // the prompt as necessary.
    if (is_input == -1) {
      quit = 1;
    } else {
      need_prompt = is_input;
    }
  }
  printf("Exiting...\n");
  free_proc_list(processes);
  return 0;
}
