#include "parser.h"
#include <string.h>

ASTNode *parse_lista_decl_globais(Parser *p)
{
    ASTNode *root = make_node(NODE_LISTA_DECL_GLOBAIS, 0);
    while (p->current && p->current->type && strcmp(p->current->type, "TOK_EOF") != 0) {
        t_token *antes_do_erro = p->current;
        ASTNode *d = parse_declaracao_global(p);
        
        if (d) {
            add_filho(root, d);
        } else {
            // Se o ponteiro não se moveu, consome um token para evitar loop infinito
            if (p->current == antes_do_erro) {
                parser_next_token(p);
            }
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