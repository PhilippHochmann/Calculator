#include <stdio.h>
#include <string.h>

#include "cmd_evaluation.h"
#include "core.h"
#include "../util/console_util.h"
#include "../util/string_util.h"
#include "../parsing/node.h"
#include "../arithmetics/arith_context.h"
#include "../arithmetics/arith_rules.h"

bool cmd_evaluation_check(__attribute__((unused)) char *input)
{
    return true;
}

/*
Summary: The evaluation command is executed when input is no other command (hence last in command array at core.c)
*/
void cmd_evaluation_exec(char *input)
{
    Node *res;
    if (parse_input_from_console(input, "Error: %s\n", &res, true))
    {
        ConstantType result = arith_eval(res);
        printf(g_interactive ? "= " CONSTANT_TYPE_FMT "\n" : CONSTANT_TYPE_FMT "\n", result);
        update_ans(result);
        free_tree(res);
    }
}
