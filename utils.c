/* @file utils.c
 * @brief Source file for various utility functions
 */

#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* function: remove_first
 * ----------------------
 * Removes the first element of an array of strings
 * by shifting all elements to the left by one.
 * input: array of strings
 */
void remove_first(char *array[])
{
    int i = 0;
    while (array[i] != NULL)
    {
        array[i] = array[i + 1];
        i++;
    }
}

/* function: msg_on_prev_line
 * --------------------------
 * Prints a message on the previous line of the terminal.
 * input: message to print
 */
void msg_on_prev_line(char *msg)
{
    printf("\n");
    printf("\x1b[1F");
    printf("%c[2K", 27);
    printf("%s\n", msg);
}

/* function: remove_newline
 * ------------------------
 * Removes the newline character from the end of a string.
 * input: string to remove newline from
 */
void remove_newline(char *str)
{
    int i = 0;
    while (str[i] != '\0')
    {
        if (str[i] == '\n')
        {
            str[i] = '\0';
        }
        i++;
    }
}
