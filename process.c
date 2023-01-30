/* @file process.c
 * @brief Source file for process management functions
 */

#include "process.h"
#include "list.h"
#include "utils.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// ANSI color codes for printing
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_RESET "\x1b[0m"

int MAX_PATH = 100;
int MAX_LINE = 1000;
int MSG_LEN = 100;

/* function: parse_stat
 * --------------------
 * helper function for print_process. parses the stat file
 * for a process and prints the values of the following fields:
 * comm, state, utime, stime, rss, vcsw, ivcsw
 * inputs: line - the line from the stat file
 */
static void parse_stat(char *line) {
  char *token = strtok(line, " ");
  int i = 0;
  while (token != NULL) {
    switch (i) {
    // values here are 1 less than the column number in /proc/[pid]/stat
    // because the first column is the pid, which is already printed.
    case 1:
      printf("comm: %s ", token);
      break;
    case 2:
      printf("state: %s ", token);
      break;
    case 13:
      printf("utime: %s ", token);
      break;
    case 14:
      printf("stime: %s ", token);
      break;
    case 23:
      printf("rss: %s ", token);
      break;
    case 39:
      printf("vcsw: %s ", token);
      break;
    case 40:
      printf("ivcsw: %s", token);
      break;
    }
    i++;
    token = strtok(NULL, " ");
  }
  printf("\n");
}

/* function print_process
 * ----------------------
 * Retrieves the stat file for a process and prints
 * the values of the following fields:
 * comm, state, utime, stime, rss, vcsw, ivcsw
 * inputs: pid - pid of the process to print
 */
void print_process(int pid) {
  // parse /proc/[pid]/stat
  char *path = malloc(MAX_PATH);
  sprintf(path, "/proc/%d/stat", pid);
  FILE *fp = fopen(path, "r");

  if (fp == NULL) {
    printf("Error: could not open file %s", path);
    exit(1);
  }

  char line[MAX_LINE];
  fgets(line, MAX_LINE, fp);
  fclose(fp);
  free(path);
  printf("pid: %d ", pid);
  parse_stat(line);
}

/* function print_pname
 * --------------------
 * Retrieves the cmdline file for a process and prints
 * the command name.
 * inputs: pid - pid of the process to print
 */
void print_pname(pid_t pid) {
  // parse /proc/[pid]/cmdline
  char *path = malloc(MAX_PATH);
  sprintf(path, "/proc/%d/cmdline", pid);
  FILE *fp = fopen(path, "r");

  if (fp == NULL) {
    printf("Error: could not open file %s", path);
    exit(1);
  }

  char line[MAX_LINE];
  fgets(line, MAX_LINE, fp);
  fclose(fp);
  free(path);
  printf("%d: %s\n", pid, line);
}

/* function: fork_process
 * ----------------------
 * Forks a child process and executes the command specified by args.
 * If the command is invalid, the child prints an error message and exits.
 * Otherwise, the parent adds the forked child process to the list of processes.
 * Void output because errors are handled by the child process.
 * inputs: args - array of arguments to pass to execvp. The first element
 *                should be the command to execute.
 *         processes - list of processes
 */
void fork_process(char *args[], proc_list_t *processes) {
  int pid = fork();
  if (pid == 0) {
    int result = execvp(args[0], args);
    if (result == -1) {
      // execvp failed, print error message and exit.
      // prevents the child process from continuing and
      // possibly causing fork bombs.
      char msg[MSG_LEN];
      sprintf(msg, "Error: invalid command %s", args[0]);
      msg_on_prev_line(msg);
      exit(1);
    }
  } else if (pid > 0) {
    // add to list of processes. Add at the end
    // to preserve the order of execution, and because
    // it is faster under the assumption that the first process
    // to start is the first process to finish, meaning that
    // it is more likely for remove operations to be at the
    // beginning of the list.
    processes = add_at_end(processes, pid);
  } else {
    printf("Error: fork failed");
  }
}
/* function: send_signal
 * ---------------------
 * Sends a signal to a child process. Prints an error message if
 * the provided pid does not correspond to a child process.
 * In the case of a SIGKILL, also removes the process from the
 * process list.
 * inputs: processes - list of processes
 *         pid - pid of the process to send the signal to
 *         sig - signal to send
 */
void send_signal(proc_list_t *processes, int pid, int sig) {
  // makes the code that calls this function cleaner
  // if this function just returns on a negative pid.
  if (pid < 0) {
    return;
  }
  if (contains_pid(processes, pid) == 0) {
    printf("Error: Process %d doesn't exist or was not started by PMan\n", pid);
  } else {
    kill(pid, sig);
    switch (sig) {
    case SIGKILL:
      remove_by_pid(processes, pid);
      printf(ANSI_COLOR_RED "Killed process %d\n" ANSI_COLOR_RESET, pid);
      break;
    case SIGSTOP:
      printf(ANSI_COLOR_YELLOW "Stopped process %d\n" ANSI_COLOR_RESET, pid);
      break;
    case SIGCONT:
      printf(ANSI_COLOR_GREEN "Started process %d\n" ANSI_COLOR_RESET, pid);
      break;
    }
  }
}

/* function: handle_process_exit
 * -----------------------------
 * Prints a message when a process exits or is killed.
 * Removes the process from the list of processes. Basically
 * a wrapper function for common code in check_processes.
 * inputs: pid - pid of the process that exited
 *         msg - message to print
 *         processes - list of processes
 */
static void handle_process_exit(int pid, char *msg, proc_list_t *processes) {
  char o_msg[MSG_LEN];
  sprintf(o_msg, "Process %d %s", pid, msg);
  msg_on_prev_line(o_msg);
  remove_by_pid(processes, pid);
}

/* function: check_processes
 * -------------------------
 * Checks the state of the program's child processes to see if
 * any exited or been killed. Prints a message informing of any
 * such events, and removes the process from the provided process list.
 * inputs: processes - list of processes
 * returns: 1 if any process has exited or been killed
 *           0 otherwise
 */
int check_processes(proc_list_t *processes) {
  int status;
  // use WNOHANG so waitpid doesn't block
  int pid = waitpid(-1, &status, WNOHANG);
  while (pid > 0) {
    if (WIFSIGNALED(status)) {
      handle_process_exit(pid, "was killed", processes);
      return 1;
    }
    if (WIFEXITED(status)) {
      handle_process_exit(pid, "has exited", processes);
      return 1;
    }
    pid = waitpid(-1, &status, WNOHANG);
  }
  return 0;
}