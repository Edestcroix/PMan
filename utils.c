/* @file utils.c
 * @brief Source file for various utility functions
 */

#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Removes the first element of an array of strings
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

/* Prints a message on the previous line of the terminal.
 * input: message to print
 */
void msg_on_prev_line(char *msg) {
  printf("\n");
  printf("\x1b[1F");
  printf("%c[2K", 27);
  printf("%s\n", msg);
}

/* Removes the newline character from the end of a string.
 * input: string to remove newline from
 */
void remove_newline(char *str) {
  int last_char = strlen(str) - 1;
  if (str[last_char] == '\n') {
    str[last_char] = '\0';
  }
}

/* wipes input string buffer up to 'size' chars */
void clean_buffer(char *buffer, int size) { memset(buffer, 0, size); }

/* checks if a string consists of only space characters
 * inputs: str: the string to check
 * output: 1 if the string is all spaces, 0 otherwise
 */
int all_spaces(char *str) { return strlen(str) == strspn(str, " "); }

/* concatenates at most 'limit' characters from each string in str_list
 * to dest
 */
void concat_strs(char *dest, char *str_list[], int limit) {
  int i = 0;
  strncpy(dest, str_list[i++], limit);
  while (str_list[i] != NULL) {
    strncat(dest, " ", limit);
    strncat(dest, str_list[i++], limit);
  }
}
