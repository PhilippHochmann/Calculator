#include <stdio.h>
#include <string.h>

#include "help.h"
#include "core.h"
#include "command.h"
#include "util.h"
#include "../engine/operator.h"

#define TRIG_IND 18
#define MISC_FUNC_IND 30
#define CONSTANTS_IND 44

void help_init()
{

}

bool help_check(char *input)
{
    return strcmp(input, "help") == 0;
}

void print_op(Operator *op)
{
    printf(OP_COLOR);
    switch (op->placement)
    {
        case OP_PLACE_PREFIX:
            if (op->arity != 0)
            {
                printf("%sx", op->name);
            }
            else
            {
                printf("%s", op->name);
            }
            break;
            
        case OP_PLACE_INFIX:
            if (strlen(op->name) == 1)
            {
                printf("x%sy", op->name);
            }
            else
            {
                printf("x %s y", op->name);
            }
            break;
            
        case OP_PLACE_POSTFIX:
            printf("x%s", op->name);
            break;
            
        case OP_PLACE_FUNCTION:
            if (op->arity != DYNAMIC_ARITY)
            {
                printf("%s(%d)", op->name, op->arity);
            }
            else
            {
                printf("%s(*)", op->name);
            }
            break;
    }

    // Print additional information when debug mode is active
    if (g_debug && op->arity > 0 && op->placement != OP_PLACE_FUNCTION)
    {
        if (op->placement == OP_PLACE_INFIX)
        {
            char assoc = '?';
            switch (op->assoc)
            {
                case OP_ASSOC_BOTH:
                    assoc = 'B';
                    break;
                case OP_ASSOC_LEFT:
                    assoc = 'L';
                    break;
                case OP_ASSOC_RIGHT:
                    assoc = 'R';
                    break;
            }

            printf(COL_RESET "%d,%c ", op->precedence, assoc);
        }
        else
        {
            printf(COL_RESET "%d ", op->precedence);
        }
        
    }
    else
    {
        printf(COL_RESET " ");
    }
}

void help_exec(ParsingContext *ctx, __attribute__((unused)) char *input)
{
#ifdef DEBUG
    printf("Calculator %s Debug build (c) 2019, Philipp Hochmann\n", VERSION);
#else
    printf("Calculator %s (c) 2019, Philipp Hochmann\n", VERSION);
#endif
    printf("Commands: debug, help, rules, <function> := <after>, <before> -> <after>, load <path>\n");

    printf("\nBasic operators:\n");
    for (size_t i = 0; i < TRIG_IND; i++)
    {
        print_op(&ctx->operators[i]);
    }

    printf("\nTrigonometric functions:\n");
    for (size_t i = TRIG_IND; i < MISC_FUNC_IND; i++)
    {
        print_op(&ctx->operators[i]);
    }

    printf("\nMiscellaneous functions:\n");
    for (size_t i = MISC_FUNC_IND; i < CONSTANTS_IND; i++)
    {
        print_op(&ctx->operators[i]);
    }

    printf("\nConstants:\n");
    for (size_t i = CONSTANTS_IND; i < ctx->num_ops; i++)
    {
        print_op(&ctx->operators[i]);
    }

    printf("\n(%zu available operators)\n", ctx->num_ops);
}
