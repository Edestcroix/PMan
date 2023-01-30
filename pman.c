#include "list.h"
#include "process.h"
#include "utils.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

int MAX_INPUT = 1000; // max chars to read from stdin
int MAX_ARGS = 100;   // max amount of unique arguments to read
int WAIT_TIME = 1;    // how often select should check for input, in seconds
int CMD = 0;          // position of the command to handle by PMan in the args list
int FIRST_ARG = 1;    // position of the first argument in the args list

/* function parse_input
 * -------------------
 * parses the input string into an array of arguments
 * assumes input string uses spaces to separate commands/arguments
 * inputs: input - the input string
 *         args - the array of arguments
 */
void parse_input(char *input, char *args[]) {
  char *token = strtok(input, " ");
  int i = 0;
  while (token != NULL) {
    args[i] = token;
    i++;
    token = strtok(NULL, " ");
  }
  args[i] = NULL;
}

/* function: list_processes
 * ------------------------
 * prints the list of background processes
 * inputs: processes - the list of background processes
 */
void list_processes(proc_list_t *processes) {
  if (processes->size > 1) {
    printf("Background processes (%d):\n", processes->size);
  } else if (processes->size == 1) {
    printf("Background process (1):\n");
  } else {
    printf("No background processes\n");
    return;
  }
  proc_t *cur = processes->head;
  while (cur != NULL) {
    print_pname(cur->pid);
    cur = cur->next;
  }
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
 * returns: 0 if the command was handled,
 *         -1 if the command was quit or exit
 */
int handle_cmds(char *args[], proc_list_t *processes) {
  char *cmd = args[CMD];
  if (strcmp(cmd, "bg") == 0) {
    // remove the "bg" command from args. Everything after are
    // the command/arguments for the child process to execute.
    remove_first(args);
    fork_process(args, processes);

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
      print_process(pid);

  } else if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "exit") == 0) {
    return -1;

  } else {
    printf("Invalid command \"%s\"\n", cmd);
  }
  return 0;
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
  char input[MAX_INPUT];
  char *args[MAX_ARGS];
  proc_list_t *processes = create_list();

  fd_set readfds;
  struct timeval tv;

  // main event loop
  while (!quit) {
    // set up values for select
    FD_ZERO(&readfds);
    FD_SET(fileno(stdin), &readfds);
    tv.tv_sec = WAIT_TIME;
    tv.tv_usec = 0;

    // print prompt only if needed, preventing printing multiple prompts
    // every loop iteration and allowing finer control over displaying prompt
    if (need_prompt) {
      printf("PMan: > ");
      need_prompt = 0;
      // flush input when displaying prompt because process exit messages
      // can cause unexpected behaviour when they are printed over user input.
      int in = dup(STDIN_FILENO);
      tcdrain(in);
      tcflush(in, TCIFLUSH);
      close(in);
    }
    fflush(stdout);

    // use select to check if input is available
    int readable = select(fileno(stdin) + 1, &readfds, NULL, NULL, &tv);
    if (readable == -1) {
      perror("select");
      exit(1);
    } else if (readable == 0) {
      // check_processes returns 1 if a process has exited and 0 otherwise.
      // Since check_processes prints a message when a child has exited,
      // updating need_prompt will trigger a reprint of the prompt when needed
      need_prompt = check_processes(processes);
    } else {
      need_prompt = 1;
      // retrieve the user input and process it.
      read(fileno(stdin), input, MAX_INPUT);
      remove_newline(input);
      parse_input(input, args);
      if (handle_cmds(args, processes) == -1) {
        quit = 1;
      }
    }
  }
  printf("Exiting...\n");
  free_proc_list(processes);
  return 0;
}
