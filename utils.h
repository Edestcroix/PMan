#ifndef _UTLS_H_
#define _UTLS_H_

void remove_first(char *args[]);
void msg_on_prev_line(char *msg);
void remove_newline(char *input);
int all_spaces(char *str);
void concat_strs(char *dest, char *str_list[], int limit);
void clean_buffer(char *buffer, int size);
//void path_search(char *executable, char *result);

#endif
