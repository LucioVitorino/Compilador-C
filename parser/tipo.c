#include "parser.h"

ASTNode *parse_especificador_tipo(Parser *p)
{
    if (!p->current) return NULL;
    if (p->current->type && (strcmp(p->current->type, "KEYWORD_INT") == 0 || strcmp(p->current->type, "KEYWORD_FLOAT") == 0 || strcmp(p->current->type, "KEYWORD_CHAR") == 0 || strcmp(p->current->type, "KEYWORD_VOID") == 0)) {
        const char *val = p->current->value ? p->current->value : "";
        int line = p->current->line;
        parser_next_token(p);
        return make_folha(NODE_ESPECIFICADOR_TIPO, val, line);
    }
    return NULL;
}
