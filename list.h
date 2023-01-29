/* @file list.h
 * @brief Header file for linked list functions to store process ids
 */

#include <sys/types.h>

#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_

typedef struct proc_t
{
    pid_t pid;
    struct proc_t *next;
    

} proc_t;

typedef struct proc_list_t
{
    int size;
    proc_t *head;
    proc_t *tail;

} proc_list_t;

proc_list_t *create_list();
proc_list_t *add_at_end(proc_list_t *proc_list, int pid);
proc_list_t *remove_by_pid(proc_list_t *proc_list, int pid);
int contains_pid(proc_list_t *proc_list, int pid);
void free_proc_list(proc_list_t *proc_list);

#endif