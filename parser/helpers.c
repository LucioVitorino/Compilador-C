#include "parser.h"

static int parse_asteriscos_internal(Parser *p)
{
    int count = 0;
    while (p->current && p->current->value && strcmp(p->current->value, "*") == 0) {
        parser_next_token(p);
        count++;
    }
    return count;
}

static int parse_sufixo_array_opcional_internal(Parser *p)
{
    int seen = 0;
    while (p->current && p->current->value && strcmp(p->current->value, "[") == 0) {
        // consume [ expr ]
        parser_next_token(p);
        parse_expressao(p);
        if (p->current && p->current->value && strcmp(p->current->value, "]") == 0) parser_next_token(p);
        seen = 1;
    }
    return seen;
}

/* parametro := especificador_tipo resto_parametro
   resto_parametro := asteriscos IDENTIFIER sufixo_array_opcional | epsilon
*/
static ASTNode *parse_parametro_internal(Parser *p)
{
    ASTNode *tipo = parse_especificador_tipo(p);
    if (!tipo) return NULL;

    int stars = parse_asteriscos_internal(p);

    ASTNode *param = make_node(NODE_DECLARACAO_VARIAVEL, 0);
    if (p->current && p->current->type && strcmp(p->current->type, "IDENTIFIER") == 0) {
        param->valor = strdup(p->current->value);
        parser_next_token(p);
    }
    // possible array suffix
    parse_sufixo_array_opcional_internal(p);
    // attach type as child
    add_filho(param, tipo);
    (void)stars;
    return param;
}

static ASTNode *parse_lista_parametros_internal(Parser *p)
{
    ASTNode *root = make_node(NODE_LISTA_DECL_GLOBAIS, 0);
    ASTNode *first = parse_parametro_internal(p);
    if (!first) return NULL;
    add_filho(root, first);
    while (p->current && p->current->value && strcmp(p->current->value, ",") == 0) {
        parser_next_token(p); // consume ','
        ASTNode *pr = parse_parametro_internal(p);
        if (pr) add_filho(root, pr);
        else break;
    }
    return root;
}

/* parse_parametros_opcionais: returns NULL for empty; returns a NODE_LISTA_DECL_GLOBAIS node for parameters; handles 'void' specially */
ASTNode *parse_parametros_opcionais(Parser *p)
{
    if (!p->current) return NULL;
    if (p->current->type && strcmp(p->current->type, "KEYWORD_VOID") == 0) {
        // 'void' as only parameter (no params)
        parser_next_token(p);
        return NULL;
    }
    // if next token starts a type specifier
    if (p->current && p->current->type && (strcmp(p->current->type, "KEYWORD_INT") == 0 || strcmp(p->current->type, "KEYWORD_FLOAT") == 0 || strcmp(p->current->type, "KEYWORD_CHAR") == 0 || strcmp(p->current->type, "KEYWORD_VOID") == 0 || strcmp(p->current->type, "IDENTIFIER") == 0)) {
        return parse_lista_parametros_internal(p);
    }
    return NULL;
}

/* Expose wrappers used from other parser modules */
int parse_asteriscos(Parser *p) { return parse_asteriscos_internal(p); }
int parse_sufixo_array_opcional(Parser *p) { return parse_sufixo_array_opcional_internal(p); }
