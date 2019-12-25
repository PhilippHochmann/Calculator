#include "table.h"

size_t get_line_of_string(char *string, size_t line_index, char **out_start);
TextAlignment get_align(TextAlignment default_align, struct Cell *cell);
size_t console_strlen(char *str);
void print_table_internal(Table *table);