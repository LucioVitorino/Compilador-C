#include "parser.h"

t_token *parser_next_token(Parser *p)
{
    if (!p->current) return NULL;
    t_token *tok = p->current;
    p->current = p->current->next;
    return tok;
}

const char *parser_peek_value(Parser *p) { return p->current ? p->current->value : NULL; }
const char *parser_peek_type(Parser *p) { return p->current ? p->current->type : NULL; }
int parser_consume_if_value(Parser *p, const char *val) {
    if (p->current && p->current->value && strcmp(p->current->value, val) == 0) { parser_next_token(p); return 1; }
    return 0;
}
int parser_consume_if_type(Parser *p, const char *type) {
    if (p->current && p->current->type && strcmp(p->current->type, type) == 0) { parser_next_token(p); return 1; }
    return 0;
}

void parser_init(Parser *p, t_token *tokens)
{
    p->token_stream = tokens;
    p->current = tokens;
    parser_reset_error_count();
}

void parser_free(Parser *p)
{
    (void)p;
}