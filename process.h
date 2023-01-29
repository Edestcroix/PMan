/* @file process.h
 * @brief Header file for process management functions
 */

#include "list.h"

#ifndef _PROCESS_H_
#define _PROCESS_H_

void print_pname(int pid);
void print_process(int pid);
void fork_process(char *args[], proc_list_t *processes);
void send_signal(proc_list_t *processes, int pid, int sig);
int check_processes(proc_list_t *processes);

#endif