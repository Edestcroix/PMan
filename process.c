/* @file process.c
 * @brief Source file for process management functions
 */

// TODO: standardize comment formatting
#include "process.h"
#include "list.h"
#include "utils.h"
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

// ANSI color codes for printing
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_RESET "\x1b[0m"

int MAX_PATH = 100;
int MAX_LEN = 1000;
int MSG_LEN = 100;

/* helper function for print_process. parses the stat file
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
    // because the first column is the pid, which was already printed.
    case 1:
      printf("comm: %s ", token);
      break;
    case 2:
      printf("state: %s ", token);
      break;
    // TODO: Figure out if utime and stime are actually ever not 0
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

/*
 * Retrieves the stat file for a process and prints
 * the values of the following fields:
 * comm, state, utime, stime, rss, vcsw, ivcsw
 * inputs: pid - pid of the process to print
 */
void print_pstats(int pid) {
  // parse /proc/[pid]/stat
  char *path = malloc(MAX_PATH);
  sprintf(path, "/proc/%d/stat", pid);
  FILE *fp = fopen(path, "r");

  if (fp == NULL) {
    printf("Error: Process %d does not exist\n", pid);
    return;
  }

  char line[MAX_LEN];
  fgets(line, MAX_LEN, fp);
  fclose(fp);
  free(path);
  printf("pid: %d ", pid);
  parse_stat(line);
}

/* prints the list of background processes
 * in the format PID : COMMAND where COMMAND is
 * the command used to execute the process
 */
void list_processes(plist_t *processes) {
  if (processes->size > 1) {
    printf("Background processes (%d):\n", processes->size);
  } else if (processes->size == 1) {
    printf("Background process (1):\n");
  } else {
    printf("No background processes\n");
    return;
  }
  process_t *cur = processes->head;
  while (cur != NULL) {
    switch (cur->state) {
    case ACTIVE:
      printf(ANSI_COLOR_GREEN "  - %d: %s (Active)" ANSI_COLOR_RESET "\n",
             cur->pid, cur->name);
      break;
    case STOPPED:
      printf(ANSI_COLOR_YELLOW "  - %d: %s (Stopped)" ANSI_COLOR_RESET "\n",
             cur->pid, cur->name);
      break;
    }
    cur = cur->next;
  }
}

/* Forks a child process and executes the command specified by args.
 * If the command is invalid, the child prints an error message and exits.
 * Otherwise, the parent adds the forked child process to the list of processes.
 * Args is expected to contain the command to run at index 0, and the arguments
 * for said command at indices starting at 1. Returns the value of the fork()
 * command. "add" identifies if this is a foreground or background process
 */
int fork_process(char *args[], plist_t *processes, enum runin type) {
  int pid = fork();
  if (pid == 0) {
    int result = execvp(args[0], args);
    if (result == -1) {
      // execvp failed, print error message and exit.
      // prevents the child process from continuing and
      // possibly causing fork bombs.
      char msg[MSG_LEN];
      sprintf(msg, "Error: Invalid command \"%s\"", args[0]);
      msg_on_prev_line(msg);
      exit(-1);
    }
  } else if (pid > 0) {
    // name of the process is the command
    // and arguments used to start it.
    // it will not be longer than LINE_MAX,
    // because LINE_MAX is the maximum amount of
    // chars that are read from stdin.
    char name[LINE_MAX];
    concat_strs(name, args, LINE_MAX);
    if (type == BG) {
      process_t *new_process = new_node(pid, name, ACTIVE);
      processes = add_at_end(processes, new_process);
    }
  } else {
    printf("Error: Fork failed");
  }
  return pid;
}

/* Sends a signal to a child process. Prints an error message if
 * the provided pid does not correspond to a child process in processes.
 * In the case of a SIGKILL, also removes the process from the
 * process list.
 * inputs: - processes: list of processes
 *         - pid: pid of the process to send the signal to
 *         - sig: signal to send
 */
void send_signal(plist_t *processes, int pid, int sig) {
  // makes the code that calls this function cleaner
  // if this function just returns on a negative pid.
  if (pid < 0) {
    return;
  }

  process_t *process;
  if (contains_pid(processes, pid) == 0) {
    printf("Error: Process \"%d\" doesn't exist or was not started by PMan\n",
           pid);
  } else {
    kill(pid, sig);
    switch (sig) {
    case SIGKILL:
      remove_by_pid(processes, pid);
      printf(ANSI_COLOR_RED "Killed process %d" ANSI_COLOR_RESET "\n", pid);
      break;
    case SIGSTOP:
      printf(ANSI_COLOR_YELLOW "Stopped process %d" ANSI_COLOR_RESET "\n", pid);
      process = get_process(processes, pid);
      process->state = STOPPED;
      break;
    case SIGCONT:
      printf(ANSI_COLOR_GREEN "Started process %d" ANSI_COLOR_RESET "\n", pid);
      process = get_process(processes, pid);
      process->state = ACTIVE;
      break;
    }
  }
}

/* sends SIGKILL to all child processes in the processes list */
void kill_all(plist_t *processes) {
  process_t *cur = processes->head;
  while (cur != NULL) {
    send_signal(processes, cur->pid, SIGKILL);
    cur = cur->next;
  }
}

/* Prints an exit message for a process and removes the process from the
 * list of processes. Basically a wrapper function for common code
 * in check_processes.
 * inputs: - pid: pid of the process that exited
 *         - msg: message to print
 *         - processes - list of processes
 * returns: 1 if the pid was found in the list, 0 otherwise
 */
static int handle_process_exit(int pid, char *msg, plist_t *processes) {
  // if the child that exited is not in the processes list, it
  // means that it was killed from within PMan by bgkill and
  // as such the user has already been notified of the process' termination
  if (contains_pid(processes, pid)) {
    char o_msg[MSG_LEN];
    sprintf(o_msg, "  - Process %d %s", pid, msg);
    msg_on_prev_line(o_msg);
    remove_by_pid(processes, pid);
    return 1;
  }
  return 0;
}

/* checks the state of the program's child processes to see if
 * any have exited or been killed. Prints a message informing of any
 * such events, and removes the according process from the process list.
 * inputs: processes - list of processes
 * returns: 1 if a tracked process has exited or been killed
 *          0 otherwise
 */
int check_processes(plist_t *processes) {
  int status;
  // use WNOHANG so waitpid doesn't block
  int pid = waitpid(-1, &status, WNOHANG);
  while (pid > 0) {
    if (WIFSIGNALED(status)) {
      return handle_process_exit(pid, "was killed", processes);
    }
    if (WIFEXITED(status)) {
      return handle_process_exit(pid, "has exited", processes);
    }
    pid = waitpid(-1, &status, WNOHANG);
  }
  return 0;
}
