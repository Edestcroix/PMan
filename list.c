/* @file list.c
 * @brief Source file for linked list meant to store process ids
 */

#include "list.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* function: create_list
 * ---------------------
 * Creates and allocates memory for a new linked list
 * inputs: none
 * returns: pointer to the new linked list with size 0 and a NULL head
 */
proc_list_t *create_list()
{
    proc_list_t *list = malloc(sizeof(struct proc_list_t));
    list->size = 0;
    list->head = NULL;
    list->tail = NULL;
    return list;
}

/* function: new_node
 * ------------------
 * Creates and allocates memory for a new node
 * inputs: pid - pointer to the process id to be stored in the node
 * returns: a note_t instance that points to pid and a NULL next node
 */
static proc_t *new_node(int pid)
{
    proc_t *node = malloc(sizeof(struct proc_t));
    if (node == NULL)
    {
        fprintf(stderr, "Error: malloc failed in new_node");
        exit(1);
    }
    node->pid = pid;
    node->next = NULL;
    return node;
}

/* function: add_at_end
 * --------------------
 * Adds a new node to the end of the linked list.
 * Expects a non-NULL list, i.e one created with create_list
 * List tracks tail pointer to make this operation O(1).
 * inputs: proc_list - pointer to the linked list
 *         pid - the new process id to be added
 * returns: pointer to the head of the updated list
 */
proc_list_t *add_at_end(proc_list_t *proc_list, int pid)
{
    proc_list->size++;
    proc_t *node = new_node(pid);
    if (proc_list->head == NULL)
    {
        proc_list->head = node;
        proc_list->tail = node;
    }

    else
    {
        proc_list->tail->next = node;
        proc_list->tail = node;
    }

    return proc_list;
}

/* function: remove_by_pid
 * -----------------------
 * Removes a node from the linked list by its process id
 * inputs: proc_list - pointer to the linked list
 *         pid - the process id to be removed
 * returns: pointer to the head of the updated list
 */
proc_list_t *remove_by_pid(proc_list_t *proc_list, int pid)
{
    // if the list is empty, return it without updating size.
    if (proc_list->head == NULL)
    {
        return proc_list;
    }
    // update the list's head if the head is to be removed
    if (proc_list->head->pid == pid)
    {
        proc_t *next = proc_list->head->next;
        free(proc_list->head);
        proc_list->head = next;

        proc_list->size--;
        return proc_list;
    }
    // otherwise, search the list until pid is found and remove it.
    else
    {
        proc_t *cur = proc_list->head;
        while (cur->next != NULL)
        {
            if (cur->next->pid == pid)
            {
                proc_t *next = cur->next->next;
                free(cur->next);
                cur->next = next;

                proc_list->size--;
                return proc_list;
            }
            cur = cur->next;
        }
    }
    // if pid is not found, return the list as is.
    return proc_list;
}

/* function: contains_pid
 * ----------------------
 * Checks if a process id is in the linked list
 * inputs: proc_list - pointer to the linked list
 *         pid - the process id to be checked
 * returns: 1 if the process id is in the list, 0 otherwise
 */
int contains_pid(proc_list_t *proc_list, int pid)
{
    proc_t *cur = proc_list->head;
    while (cur != NULL)
    {
        if (cur->pid == pid)
            return 1;
        cur = cur->next;
    }
    return 0;
}

/* function: free_proc_list
 * ------------------------
 * Destroys the list and frees all allocated memory
 * inputs: proc_list - pointer to the linked list
 * returns: none
 */
void free_proc_list(proc_list_t *proc_list)
{
    proc_t *cur = proc_list->head;
    while (cur != NULL)
    {
        proc_t *next = cur->next;
        free(cur);
        cur = next;
    }
    free(proc_list);
}
