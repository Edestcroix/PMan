#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/select.h>
#include "list.h"
#include "process.h"
#include "utils.h"

int MAX_INPUT = 1000;
int MAX_ARGS = 100;
int WAIT_TIME_SEC = 1;

/* function parse_input
 * -------------------
 * parses the input string into an array of arguments
 * assumes input string uses spaces to separate commands/arguments
 * inputs: input - the input string
 *         args - the array of arguments
 */
void parse_input(char *input, char *args[])
{
    char *token = strtok(input, " ");
    int i = 0;
    while (token != NULL)
    {
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
void list_processes(proc_list_t *processes)
{
    if (processes->size > 1)
    {
        printf("Background processes (%d):\n", processes->size);
    }
    else if (processes->size == 1)
    {
        printf("Background process (1):\n");
    }
    else
    {
        printf("No background processes\n");
        return;
    }
    proc_t *cur = processes->head;
    while (cur != NULL)
    {
        print_pname(cur->pid);
        cur = cur->next;
    }
}

/* function: pid_from_args
 * -----------------------
 * wrapper function to call atoi on the first argument in args,
 * and print an error message if the argument is NULL.
 * inputs: args - the arguments passed to PMan
 * returns: the pid if it is valid, -1 otherwise.
 */
int pid_from_args(char *args[])
{
    if (args[0] == NULL)
    {
        printf("Error: No process id provided\n");
        return -1;
    }
    return atoi(args[0]);
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
int handle_cmds(char *args[], proc_list_t *processes)
{
    char *cmd = args[0];
    // don't want to pass the PMan command to the forked process
    remove_first(args);
    if (strcmp(cmd, "bg") == 0)
    {
        fork_process(args, processes);
    }
    else if (strcmp(cmd, "bglist") == 0)
    {
        list_processes(processes);
    }
    else if (strcmp(cmd, "bgkill") == 0)
    {
        // if the pid is invalid, pid_from_args will print an error message,
        // and return -1. send_signal just returns immediately in this case.
        send_signal(processes, pid_from_args(args), SIGKILL);
    }
    else if (strcmp(cmd, "bgstop") == 0)
    {
        send_signal(processes, pid_from_args(args), SIGSTOP);
    }
    else if (strcmp(cmd, "bgstart") == 0)
    {
        send_signal(processes, pid_from_args(args), SIGCONT);
    }
    else if (strcmp(cmd, "pstat") == 0)
    {
        int pid = pid_from_args(args);
        if (pid != -1)
            print_process(pid);
    }
    else if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "exit") == 0)
    {
        return -1;
    }
    else
    {
        printf("Invalid command: %s\n", cmd);
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
int main()
{
    char input[MAX_INPUT];
    proc_list_t *processes = create_list();
    char *args[MAX_ARGS];
    int need_prompt = 1;
    struct timeval tv;
    fd_set readfds;
    int quit = 0;

    while (!quit)
    {
        // set up values for select
        FD_ZERO(&readfds);
        FD_SET(fileno(stdin), &readfds);
        tv.tv_sec = WAIT_TIME_SEC;
        tv.tv_usec = 0;

        // print prompt if needed, prevents printing multiple prompts
        // every loop iteration, and allows finer control over displaying prompt
        if (need_prompt)
        {
            printf("PMan: > ");
            need_prompt = 0;
        }
        fflush(stdout);

        // use select to check if input is available
        int readable = select(fileno(stdin) + 1, &readfds, NULL, NULL, &tv);
        if (readable == -1)
        {
            perror("select");
            exit(1);
        }
        else if (readable == 0)
        {
            // check_processes returns 1 if a process has exited,
            // and 0 otherwise. Setting need_prompt to this value,
            // will reprint the prompt after the message from check_processes.
            need_prompt = check_processes(processes);
        }
        else
        {
            // if input is available, read and parse it, then handle commands.
            need_prompt = 1;
            read(fileno(stdin), input, MAX_INPUT);
            remove_newline(input);
            parse_input(input, args);
            if (handle_cmds(args, processes) == -1)
            {
                quit = 1;
            }
        }
    }
    printf("Exiting...\n");
    free_proc_list(processes);
    return 0;
}