#include "list.h"
#include "process.h"
#include "utils.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>

int MAX_ARGS = 100; // max amount of unique arguments to read
int WAIT_TIME = 1;  // how often select should check for input, in seconds
int CMD = 0;       // position of the command to handle by PMan in the args list
int FIRST_ARG = 1; // position of the first argument in the args list

// tracks the pid of whatever child is running in the foreground.
static sig_atomic_t fg_pid = -1;

/* passess signal sent to parent to foreground child signified by fg_pid.
 * if fg_pid is -1, then there is no foreground child and parent should exit()
 * prevents ctrl-c from terminating PMan when a foreground processes is running.
 */
void sig_handler(int sig) { (fg_pid != -1) ? kill(fg_pid, sig) : exit(0); }

/* parses the input string into an array of commands/arguments
 * assumes input string uses spaces to separate commands/arguments
 * inputs: input - the input string
 *         args - the array of argumentso
 */
void parse_cmds(char *input, char *args[]) {
  char *token = strtok(input, " ");
  int i = 0;
  while (token != NULL) {
    args[i++] = token;
    token = strtok(NULL, " ");
  }
  args[i] = NULL;
}

/* wrapper function to handle common code for all command
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

/* handles the commands that PMan can execute, and calls the appropriate
 * functions to handle them. Determines the requested command by parsing
 * the first element of args[].
 * outputs -1 if the command was quit or exit, 1 otherwise
 */
int handle_cmds(char *args[], plist_t *processes) {
  char *cmd = args[CMD];
  if (strcmp(cmd, "bg") == 0) {
    if (args[FIRST_ARG] == NULL) {
      printf("Error: Expected arguments\n");
    } else {
      // remove the "bg" command from args. Everything after are
      // the command/arguments for the child process to execute.
      remove_first(args);
      fork_process(args, processes, BG);
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
    // if no builtin commands were found, treat
    // the input as a system command and fork a
    // new process, but block waiting for the child to to terminate.
    int status;
    // while fg_pid is set, incoming SIGINTs will exit the child.
    fg_pid = fork_process(args, processes, FG);
    waitpid(fg_pid, &status, 0);
    // once child exits, reset fg_pid so SIGINTS will exit the parent.
    fg_pid = -1;
  }
  return 1;
}

/* uses select() to determine if input can be read from stdin
 * returns 1 if input can be read, 0 if not, and -1 if select()
 * fails.
 */
int is_input() {
  struct timeval tv = {.tv_sec = WAIT_TIME, .tv_usec = 0};
  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(fileno(stdin), &readfds);
  return select(fileno(stdin) + 1, &readfds, NULL, NULL, &tv);
}

/* when called, checks for user input. If input exists, it parses the
 * input and handles the parsed commands. outputs -1 if input is quit or
 * exit, 0 if no actions taken, 1 if actions were taken,
 * or terminates with an error if the input check fails */
int check_input(plist_t *processes) {
  char raw_input[LINE_MAX], input[LINE_MAX];
  char *args[MAX_ARGS];
  int child_ended = 0;

  switch (is_input()) {
  case -1:
    perror("select");
    exit(1);
  case 0:
    // no input, nothing to do.
    return 0;
  default:
    // read from stdin, doesn't block. Reads at
    // most the max line size defined by <limits.h>
    read(fileno(stdin), raw_input, LINE_MAX);

    // wipe input so previous input doesn't persist
    // when blank input is submited.
    clean_buffer(input, LINE_MAX);
    // parse the raw buffer, stripping tabs and newlines.
    sscanf(raw_input, "%[^\t\n]", input);

    if (!all_spaces(input)) {
      parse_cmds(input, args);
      return handle_cmds(args, processes);
    } else {
      printf("Error: Expected input\n");
    }
    return 1;
  }
}

/*
 * main function for PMan. Initializes child processes list, links signal
 * handlers; handles the main loop of the program, printing input prompts,
 * checking for user input and whether children have terminated.
 */
int main() {
  int quit = 0, need_prompt = 1;
  plist_t *processes = create_list();

  signal(SIGINT, sig_handler);

  // main event loop
  while (!quit) {
    if (need_prompt) {
      printf("PMan: > ");
      need_prompt = 0;
    }
    // flushing stdout here makes displaying output faster.
    fflush(stdout);
    int is_input = check_input(processes);
    if (is_input == -1) {
      quit = 1;
    } else {
      // check_input returns 1 whenever input was submitted
      // in which case a new prompt is needed. Additionally,
      // a new prompt is also needed whenever check_processes()
      // returns 1 as well, which is when it prints a termination message.
      need_prompt = is_input || check_processes(processes);
    }
  }
  kill_all(processes);
  free_list(processes);
  printf("Exiting...\n");
  exit(0);
}
