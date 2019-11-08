#include <stdbool.h>
#include "tree/parser.h"

bool is_space(char c);
bool is_digit(char c);
bool is_letter(char c);
size_t ansi_strlen(char *str);
bool begins_with(char *prefix, char *str);
void trim(char *str);
char *perr_to_string(ParserError perr);
