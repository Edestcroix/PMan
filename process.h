/* @file process.h
 * @brief Header file for process management functions
 */

#include "list.h"

#ifndef _PROCESS_H_
#define _PROCESS_H_

// identifies where to run a processes: ForeGround(FG) or BackGround(BG)
enum runin { FG, BG };

void print_process(int pid);
void print_pstats(int pid);
void list_processes(plist_t *processes);
int fork_process(char *args[], plist_t *processes, enum runin type);
void send_signal(plist_t *processes, int pid, int sig);
void kill_all(plist_t *processes);
int check_processes(plist_t *processes);

#endif
