/* @file list.h
 * @brief Header file for linked list to store processes
 */

#include <limits.h>
#include <sys/types.h>

#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_

enum pstate { ACTIVE, STOPPED };

// node of the list, describes a process with
// pid, the current state, (active or stopped) and
// the name, which is the command entered into pman to
// start the process.
typedef struct process_t {
  pid_t pid;
  enum pstate state;
  char name[LINE_MAX];
  struct process_t *next;

} process_t;

// list struct, holds the head of a list and the number of
// elements in the list.
typedef struct plist_t {
  int size;
  process_t *head;
  process_t *tail;

} plist_t;

plist_t *create_list();
process_t *new_node(int pid, char name[LINE_MAX], enum pstate state);
plist_t *add_at_end(plist_t *proc_list, process_t *pnew);
plist_t *remove_by_pid(plist_t *proc_list, int pid);
int contains_pid(plist_t *proc_list, int pid);
process_t *get_process(plist_t *proc_list, int pid);
void free_list(plist_t *proc_list);

#endif
