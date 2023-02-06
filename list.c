/* @file list.c
 * @brief Source file for linked list meant to store process ids
 */

#include "list.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Creates and allocates memory for a new linked list
 * inputs: none
 * returns: pointer to the new linked list with size 0 and a NULL head
 */
plist_t *create_list() {
  plist_t *list = (plist_t *)malloc(sizeof(struct plist_t));
  list->size = 0;
  list->head = NULL;
  list->tail = NULL;
  return list;
}

/* Creates and allocates memory for a new node
 * inputs: pid - pointer to the process id to be stored in the node
 * returns: a process_t instance that points to pid and a NULL next node
 */
process_t *new_node(int pid, char name[LINE_MAX], enum pstate state) {
  process_t *node = (process_t *)malloc(sizeof(struct process_t));
  if (node == NULL) {
    fprintf(stderr, "Error: malloc failed in new_node");
    exit(1);
  }
  node->pid = pid;
  node->state = state;
  strncpy(node->name, name, LINE_MAX);
  node->next = NULL;
  return node;
}

/* Adds a new node to the end of the linked list.
 * Expects a non-NULL list, i.e one created with create_list
 * List tracks tail pointer to make this operation O(1).
 * inputs: proc_list - pointer to the linked list
 *         pid - the new process id to be added
 * returns: pointer to the head of the updated list
 */
plist_t *add_at_end(plist_t *proc_list, process_t *p_new) {
  proc_list->size++;
  if (proc_list->head == NULL) {
    proc_list->head = p_new;
    proc_list->tail = p_new;
  } else {
    proc_list->tail->next = p_new;
    proc_list->tail = p_new;
  }

  return proc_list;
}

/* Removes a node from the linked list by its process id
 * inputs: proc_list - pointer to the linked list
 *         pid - the process id to be removed
 * returns: the updated list
 */
plist_t *remove_by_pid(plist_t *proc_list, int pid) {
  // update the list's head if the head is to be removed
  if (proc_list->head != NULL && proc_list->head->pid == pid) {
    process_t *next = proc_list->head->next;
    free(proc_list->head);
    proc_list->head = next;
    proc_list->size--;
    return proc_list;
  }
  // otherwise, search the list until pid is found and remove it.
  else {
    process_t *cur = proc_list->head;
    while (cur->next != NULL) {
      if (cur->next->pid == pid) {
        process_t *next = cur->next->next;
        free(cur->next);
        cur->next = next;
        // critical to update the tail pointer if the tail is removed,
        // otherwise anything added to the end will be lost.
        if (cur->next == NULL)
          proc_list->tail = cur;
        proc_list->size--;
        return proc_list;
      }
      cur = cur->next;
    }
  }
  return proc_list;
}

/* Checks if a process id is in the linked list
 * inputs: proc_list - pointer to the linked list
 *         pid - the process id to be checked
 * returns: 1 if the process id is in the list, 0 otherwise
 */
int contains_pid(plist_t *proc_list, int pid) {
  process_t *cur = proc_list->head;
  while (cur != NULL) {
    if (cur->pid == pid)
      return 1;
    cur = cur->next;
  }
  return 0;
}

process_t *get_process(plist_t *proc_list, int pid) {
  process_t *cur = proc_list->head;
  while (cur != NULL) {
    if (cur->pid == pid)
      return cur;
    cur = cur->next;
  }
  return NULL;
}

/* Destroys the list and frees all allocated memory
 * inputs: proc_list - pointer to the linked list
 */
void free_list(plist_t *proc_list) {
  process_t *cur = proc_list->head;
  while (cur != NULL) {
    process_t *next = cur->next;
    free(cur);
    cur = next;
  }
  free(proc_list);
}
