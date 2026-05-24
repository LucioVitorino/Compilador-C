#include "parser.h"

ASTNode *parse_lista_decl_globais(Parser *p)
{
    ASTNode *root = make_node(NODE_LISTA_DECL_GLOBAIS, 0);
    while (p->current && p->current->type && strcmp(p->current->type, "TOK_EOF") != 0) {
        ASTNode *d = parse_declaracao_global(p);
        if (d) {
            add_filho(root, d);
        } else {
            /* avoid infinite loop: consume token if no parse */
            if (p->current) parser_next_token(p);
            else break;
        }
    }
    return root;
}

ASTNode *parse_programa(Parser *p)
{
    ASTNode *root = make_node(NODE_PROGRAMA, 0);
    ASTNode *list = parse_lista_decl_globais(p);
    if (list) add_filho(root, list);
    return root;
}
