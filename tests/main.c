#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../src/util/string_builder.h"
#include "../src/table/table.h"
#include "../src/client/commands/commands.h"
#include "../src/client/version.h"

#include "test.h"
#include "test_tree_util.h"
#include "test_parser.h"
#include "test_tree_to_string.h"
#include "test_randomized.h"
#include "test_table.h"
#include "test_simplification.h"
#include "test_data_structures.h"

#define FUZZER_SEED 21

/*
These tests should not have any memory leaks when they are passed
Please check with valgrind when all tests pass
Memory leaks are intentionally present when tests fail (for brevity)
*/

static const size_t NUM_TESTS = 7;
static Test (*test_getters[])() = {
    get_tree_util_test,
    get_parser_test,
    get_tree_to_string_test,
    get_randomized_test,
    get_table_test,
    get_simplification_test,
    get_data_structures_test
};

int main()
{
    srand(FUZZER_SEED);
    init_commands();
    Table *table = get_empty_table();
    set_default_alignments(table, 4,
        (TableHAlign[]){ H_ALIGN_RIGHT, H_ALIGN_LEFT, H_ALIGN_RIGHT, H_ALIGN_LEFT }, NULL);
    add_empty_cell(table);
    override_left_border(table, BORDER_NONE);
    add_cell(table, " Test suite ");
    add_cell(table, " Result ");
    override_horizontal_alignment_of_row(table, H_ALIGN_LEFT);
    next_row(table);
    set_hline(table, BORDER_SINGLE);

    bool error = false;
    StringBuilder error_builder = strbuilder_create(100);
    for (size_t i = 0; i < NUM_TESTS; i++)
    {
        Test test = test_getters[i]();
        add_cell_fmt(table, " %zu ", i + 1);
        add_cell_fmt(table, " %s ", test.name);

        if (test.suite(&error_builder))
        {
            add_cell(table, F_GREEN " passed " COL_RESET);
        }
        else
        {
            printf("[" F_RED "%s" COL_RESET "] %s",
                test_getters[i]().name,
                (char*)error_builder.buffer);
            add_cell(table, F_RED " failed " COL_RESET);
            error = true;
        }
        strbuilder_clear(&error_builder);
        next_row(table);
    }
    vec_destroy(&error_builder);

    set_span(table, 2, 1);
    override_horizontal_alignment(table, H_ALIGN_CENTER);
    set_hline(table, BORDER_SINGLE);
    add_cell(table, " End result ");
    add_cell(table, error ? F_RED " failed " COL_RESET : F_GREEN " passed " COL_RESET);
    next_row(table);
    make_boxed(table, BORDER_SINGLE);
    print_table(table);
    free_table(table);
    unload_commands();
    printf(COPYRIGHT_NOTICE);

    return error ? EXIT_FAILURE : EXIT_SUCCESS;
}
