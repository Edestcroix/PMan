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
void remove_first(char *array[]) {
  int i = 0;
  while (array[i] != NULL) {
    array[i] = array[i + 1];
    i++;
  }
}

/* function: msg_on_prev_line
 * --------------------------
 * Prints a message on the previous line of the terminal.
 * input: message to print
 */
void msg_on_prev_line(char *msg) {
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
void remove_newline(char *str) {
  int last_char = strlen(str) - 1;
  if (str[last_char] == '\n') {
    str[last_char] = '\0';
  }
}

/* function: all_spaces
 * checks if a string consists of only space characters
 * inputs: - str: the string to check
 * output: - 1 if the string is all spaces, 0 otherwise 
*/
int all_spaces(char *str) {
  int len = strlen(str);
  int len_space = strspn(str, " ");
  return len == len_space;
}
