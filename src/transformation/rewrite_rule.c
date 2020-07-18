#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "../util/string_util.h"
#include "../util/console_util.h"
#include "../tree/tree_util.h"
#include "../core/evaluation.h"
#include "rewrite_rule.h"
#include "matching.h"

#define NORMAL_VAR_ID  0
#define RULE_VAR_ID    1

#define RULESET_KEYWORD "Ruleset"
#define ARROW           " -> "
#define COMMENT_PREFIX  '\''

#define MAX_RULESET_ITERATIONS 1000 // To protect against endless loops

void mark_vars(Node *tree, char id)
{
    char *vars[MAX_MAPPED_VARS];
    size_t num_vars = list_variables(tree, vars);
    size_t safe_var_count = count_variables(tree, false);
    for (size_t i = 0; i < num_vars; i++)
    {
        Node **nodes[safe_var_count];
        size_t num_nodes = get_variable_nodes(&tree, vars[i], nodes);
        for (size_t j = 0; j < num_nodes; j++)
        {
            set_id(*(nodes[j]), id);
        }
    }
}

/*
Summary: Constructs new rule. Warning: "before" and "after" are not copied, so don't free them!
*/
RewriteRule get_rule(Node *before, Node *after, MappingFilter filter)
{
    mark_vars(before, RULE_VAR_ID);
    mark_vars(after, RULE_VAR_ID);
    return (RewriteRule){
        .before = before,
        .after  = after,
        .filter = filter
    };
}

/*
Summary: Frees trees "before" and "after"
*/
void free_rule(RewriteRule rule)
{
    free_tree(rule.before);
    free_tree(rule.after);
}

/*
Summary: Substitutes subtree in which matching was found according to rule
*/
void transform_matched_by_rule(Node *rule_after, Matching *matching, Node **matched_subtree)
{
    if (rule_after == NULL || matching == NULL) return;
    
    Node *transformed = tree_copy(rule_after);
    for (size_t i = 0; i < matching->num_mapped; i++)
    {
        replace_variable_nodes_by_list(&transformed, matching->mapped_nodes[i], matching->mapped_vars[i], RULE_VAR_ID);
    }
    mark_vars(transformed, NORMAL_VAR_ID);
    tree_replace(matched_subtree, transformed);
}

/*
Summary: Tries to find matching in tree and directly transforms tree by it
Returns: True when matching could be applied, false otherwise
*/
bool apply_rule(Node **tree, RewriteRule *rule)
{
    Matching matching;
    
    // Try to find matching in tree with pattern specified in rule
    Node **res = find_matching(tree, rule->before, &matching, rule->filter);
    if (res == NULL) return false;
    // If matching is found, transform tree with it
    transform_matched_by_rule(rule->after, &matching, res);

    return true;
}

Vector get_empty_ruleset()
{
    return vec_create(sizeof(RewriteRule), 1);
}

void add_to_ruleset(Vector *rules, RewriteRule rule)
{
    VEC_PUSH_ELEM(rules, RewriteRule, rule);
}

void free_ruleset(Vector *rules)
{
    for (size_t i = 0; i < vec_count(rules); i++)
    {
        free_rule(VEC_GET_ELEM(rules, RewriteRule, i));
    }
    vec_destroy(rules);
}

/*
Summary: Tries to apply rules (priorized by order) until no rule can be applied any more
    Guarantees to terminate after MAX_RULESET_ITERATIONS rule appliances
*/
//#include "../tree/tree_to_string.h"
int apply_ruleset(Node **tree, Vector *rules)
{
    size_t counter = 0;
    while (true)
    {
        bool applied_flag = false;
        for (size_t j = 0; j < vec_count(rules); j++)
        {
            if (apply_rule(tree, (RewriteRule*)vec_get(rules, j)))
            {
                //printf("Applied rule %zu: %s\n", j, tree_to_string(*tree, true));
                applied_flag = true;
                counter++;
                break;
            }
        }
        if (!applied_flag || counter == MAX_RULESET_ITERATIONS)
        {
            return counter;
        }
    }
    return 0; // To make compiler happy
}

bool parse_rulesets(FILE *file,
    ParsingContext *ctx,
    MappingFilter default_filter,
    size_t buffer_size,
    size_t *out_num_rulesets,
    Vector *out_rulesets)
{
    ssize_t ruleset_index = -1;

    char *line = malloc(30);
    size_t line_len = 0;
    size_t line_no = 0;
    while (getline(&line, &line_len, file) != EOF)
    {
        line_no++;
        line[strlen(line) - 1] = '\0';

        if (begins_with(RULESET_KEYWORD, line))
        {
            ruleset_index++;
            if (ruleset_index == (ssize_t)buffer_size) break;
            continue;
        }

        if (line[0] == COMMENT_PREFIX || line[0] == '\0' || line[0] == EOF)
        {
            continue;
        }

        char *right = strstr(line, ARROW);
        if (right == NULL)
        {
            report_error("Ruleset file syntax error in line %zu.\n", line_no);
            goto error;
        }

        right[0] = '\0';
        right += strlen(ARROW);
        Node *left_n = parse_conveniently(ctx, line);
        if (left_n == NULL)
        {
            report_error("Ruleset file syntax error in left side of rule in line %zu.\n", line_no);
            goto error;
        }
        Node *right_n = parse_conveniently(ctx, right);
        if (right_n == NULL)
        {
            report_error("Ruleset file syntax error in right side of rule in line %zu.\n", line_no);
            free_tree(left_n);
            goto error;
        }

        add_to_ruleset(&out_rulesets[ruleset_index], get_rule(left_n, right_n, default_filter));
    }

    for (ssize_t i = 0; i < ruleset_index; i++)
    {
        vec_trim(&out_rulesets[i]);
    }    
    *out_num_rulesets = ruleset_index + 1;
    free(line);
    return true;

    error:
    free(line);
    for (ssize_t i = 0; i < ruleset_index; i++)
    {
        free_ruleset(&out_rulesets[i]);
    }
    return false;
}
