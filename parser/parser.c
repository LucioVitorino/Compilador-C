#include "parser.h"

const char *parser_peek_value(Parser *p) { return p->current ? p->current->value : NULL; }
const char *parser_peek_type(Parser *p) { return p->current ? p->current->type : NULL; }
int parser_consume_if_value(Parser *p, const char *val) {
    if (p->current && p->current->value && strcmp(p->current->value, val) == 0) { parser_next_token(p); return 1; }
    return 0;
}
int parser_consume_if_type(Parser *p, const char *type) {
    if (p->current && p->current->type && strcmp(p->current->type, type) == 0) 
    {
         parser_next_token(p); 
        return 1; }
    return 0;
}

/* match: consume token if matches value-type string (simple) */
static int match_type(Parser *p, const char *type)
{
    if (!p->current) return 0;
    if (p->current->type && strcmp(p->current->type, type) == 0) {
        parser_next_token(p);
        return 1;
    }
    return 0;
}

ASTNode *parse_programa(Parser *p)
{
    ASTNode *root = make_node(NODE_PROGRAMA, 0);
    ASTNode *list = parse_lista_decl_globais(p);
    if (list) add_filho(root, list);
    return root;
}

void parser_free(Parser *p)
{
    (void)p;
}

/* Expressions: recursive descent simple */
ASTNode *parse_primario(Parser *p)
{
    if (!p->current) return NULL;
    if (p->current->type && (strcmp(p->current->type, "INT_LITERAL") == 0 || strcmp(p->current->type, "FLOAT_LITERAL") == 0 || strcmp(p->current->type, "IDENTIFIER") == 0)) {
        const char *val = p->current->value ? p->current->value : "";
        int line = p->current->line;
        parser_next_token(p);
        return make_folha(NODE_IDENTIFICADOR, val, line);
    }
    if (parser_consume_if_value(p, "(")) {
        ASTNode *e = parse_expressao(p);
        parser_consume_if_value(p, ")");
        return e;
    }
    return NULL;
}

ASTNode *parse_multiplicativo(Parser *p)
{
    ASTNode *left = parse_primario(p);
    while (p->current && p->current->value && (strcmp(p->current->value, "*") == 0 || strcmp(p->current->value, "/") == 0 || strcmp(p->current->value, "%") == 0)) {
        char *op = strdup(p->current->value);
        parser_next_token(p);
        ASTNode *right = parse_primario(p);
        ASTNode *node = make_node(NODE_OP_BINARIO, 0);
        node->op = op;
        add_filho(node, left);
        add_filho(node, right);
        left = node;
    }
    return left;
}

ASTNode *parse_aditivo(Parser *p)
{
    ASTNode *left = parse_multiplicativo(p);
    while (p->current && p->current->value && (strcmp(p->current->value, "+") == 0 || strcmp(p->current->value, "-") == 0)) {
        char *op = strdup(p->current->value);
        parser_next_token(p);
        ASTNode *right = parse_multiplicativo(p);
        ASTNode *node = make_node(NODE_OP_BINARIO, 0);
        node->op = op;
        add_filho(node, left);
        add_filho(node, right);
        left = node;
    }
    return left;
}

ASTNode *parse_expressao(Parser *p)
{
    return parse_aditivo(p);
}

/* type specifier: int|float|char|void */
ASTNode *parse_especificador_tipo(Parser *p)
{
    if (!p->current) return NULL;
    if (p->current->type && (strcmp(p->current->type, "KEYWORD_INT") == 0 || strcmp(p->current->type, "KEYWORD_FLOAT") == 0 || strcmp(p->current->type, "KEYWORD_CHAR") == 0 || strcmp(p->current->type, "KEYWORD_VOID") == 0)) {
        const char *val = p->current->value ? p->current->value : "";
        int line = p->current->line;
        parser_next_token(p);
        return make_folha(NODE_IDENTIFICADOR, val, line);
    }
    return NULL;
}

/* helpers */
static int parse_asteriscos(Parser *p)
{
    int count = 0;
    while (p->current && p->current->value && strcmp(p->current->value, "*") == 0) {
        parser_next_token(p);
        count++;
    }
    return count;
}

static int parse_sufixo_array_opcional(Parser *p)
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

void parse_parametros_opcionais(Parser *p)
{
    // accept 'void' or parameter list or empty
    if (p->current && p->current->type && strcmp(p->current->type, "KEYWORD_VOID") == 0) {
        parser_next_token(p);
        return;
    }
    // naive: skip until )
    while (p->current && p->current->value && strcmp(p->current->value, ")") != 0) {
        parser_next_token(p);
    }
}

ASTNode *parse_declaracao_global(Parser *p)
{
    // preprocessor directive
    if (p->current && p->current->type && strcmp(p->current->type, "PREPROCESSOR_DIRECTIVE") == 0) {
        const char *val = p->current->value ? p->current->value : "";
        int line = p->current->line;
        parser_next_token(p);
        ASTNode *n = make_node(NODE_DIRETIVA_INCLUDE, line);
        n->valor = strdup(val);
        return n;
    }

    // typedef
    if (p->current && p->current->type && strcmp(p->current->type, "KEYWORD_TYPEDEF") == 0) {
        parser_next_token(p);
        ASTNode *tipo = parse_especificador_tipo(p);
        parse_asteriscos(p);
        if (p->current && p->current->type && strcmp(p->current->type, "IDENTIFIER") == 0) {
            const char *name = p->current->value ? p->current->value : "";
            int line = p->current->line;
            parser_next_token(p);
            // skip optional array and semicolon
            parse_sufixo_array_opcional(p);
            parser_consume_if_value(p, ";");
            ASTNode *n = make_node(NODE_DECLARACAO_TYPEDEF, line);
            n->valor = strdup(name);
            if (tipo) add_filho(n, tipo);
            return n;
        }
    }

    /* general declaration: type ... */
    ASTNode *tipo = parse_especificador_tipo(p);
    if (!tipo) return NULL;
    // pointers
    int stars = parse_asteriscos(p);
    if (!p->current) return NULL;
    if (p->current->type && strcmp(p->current->type, "IDENTIFIER") == 0) {
        const char *name = p->current->value ? p->current->value : "";
        int line = p->current->line;
        parser_next_token(p);
        // function? '(' => parse params and block
        if (p->current && p->current->value && strcmp(p->current->value, "(") == 0) {
            parser_next_token(p); // consume '('
            parse_parametros_opcionais(p);
            parser_consume_if_value(p, ")");
            // optional block
            if (p->current && p->current->value && strcmp(p->current->value, "{") == 0) {
                // skip block naive
                parser_next_token(p);
                int depth = 1;
                while (p->current && depth > 0) {
                    if (p->current->value && strcmp(p->current->value, "{") == 0) { parser_next_token(p); depth++; }
                    else if (p->current->value && strcmp(p->current->value, "}") == 0) { parser_next_token(p); depth--; }
                    else parser_next_token(p);
                }
            }
            ASTNode *fn = make_node(NODE_DECLARACAO_FUNCAO, line);
            fn->valor = strdup(name);
            add_filho(fn, tipo);
            return fn;
        } else {
            // first declarator
            parse_sufixo_array_opcional(p);
            if (p->current && p->current->value && strcmp(p->current->value, "=") == 0) {
                parser_next_token(p);
                parse_expressao(p);
            }

            ASTNode *decl_list = make_node(NODE_LISTA_DECL_GLOBAIS, line);
            ASTNode *var = make_node(NODE_DECLARACAO_VARIAVEL, line);
            var->valor = strdup(name);
            add_filho(var, tipo);
            add_filho(decl_list, var);

            // more declarators separated by commas
            while (p->current && p->current->value && strcmp(p->current->value, ",") == 0) {
                parser_next_token(p); // consume ','
                parse_asteriscos(p);
                if (p->current && p->current->type && strcmp(p->current->type, "IDENTIFIER") == 0) {
                    const char *nname = p->current->value ? p->current->value : "";
                    int nline = p->current->line;
                    parser_next_token(p);
                    parse_sufixo_array_opcional(p);
                    if (p->current && p->current->value && strcmp(p->current->value, "=") == 0) {
                        parser_next_token(p);
                        parse_expressao(p);
                    }
                    ASTNode *v = make_node(NODE_DECLARACAO_VARIAVEL, nline);
                    v->valor = strdup(nname);
                    add_filho(v, tipo);
                    add_filho(decl_list, v);
                } else {
                    // skip junk until ;
                    while (p->current && p->current->value && strcmp(p->current->value, ";") != 0) parser_next_token(p);
                    break;
                }
            }

            parser_consume_if_value(p, ";");
            return decl_list;
        }
    }
    return NULL;
}

ASTNode *parse_lista_decl_globais(Parser *p)
{
    ASTNode *root = make_node(NODE_LISTA_DECL_GLOBAIS, 0);
    while (p->current && p->current->type && strcmp(p->current->type, "TOK_EOF") != 0) {
        ASTNode *d = parse_declaracao_global(p);
        if (d) add_filho(root, d);
        else parser_next_token(p);
    }
    return root;
}
