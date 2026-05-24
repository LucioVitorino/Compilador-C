#include "parser.h"

ASTNode *parse_bloco(Parser *p)
{
    if (!p->current || !(p->current->value && strcmp(p->current->value, "{") == 0)) return NULL;
    parser_next_token(p); // consume '{'
    ASTNode *blk = make_node(NODE_BLOCO, 0);
    ASTNode *list = parse_lista_itens_bloco(p);
    if (list) add_filho(blk, list);
    parser_consume_if_value(p, "}");
    return blk;
}

// Em statements.c
ASTNode *parse_lista_itens_bloco(Parser *p)
{
    ASTNode *root = make_node(NODE_LISTA_DECL_GLOBAIS, 0);

    while (p->current && p->current->value && strcmp(p->current->value, "}") != 0 && strcmp(p->current->type, "TOK_EOF") != 0) {
        ASTNode *it = parse_item_bloco(p);
        if (it) {
            add_filho(root, it);
        } else {
            // Se chegou aqui, encontramos lixo ou uma instrução completamente corrompida
            printf("Erro Sintático na linha %d: Instrução ou item de bloco inválido.\n", 
                   p->current ? p->current->line : 0);
            parser_sincronizar(p);
        }
    }
    return root;
}

ASTNode *parse_item_bloco(Parser *p)
{
    // try declaration local
    if (p->current && p->current->type && (strcmp(p->current->type, "KEYWORD_INT") == 0 || strcmp(p->current->type, "KEYWORD_FLOAT") == 0 || strcmp(p->current->type, "KEYWORD_CHAR") == 0 || strcmp(p->current->type, "KEYWORD_VOID") == 0 || strcmp(p->current->type, "IDENTIFIER") == 0)) {
        // could be declaration or instruction starting with type
        // try declaration
        ASTNode *saved = NULL;
        saved = parse_declaracao_variavel_local(p);
        if (saved) return saved;
    }
    // otherwise instruction
    return parse_instrucao(p);
}

ASTNode *parse_declaracao_variavel_local(Parser *p)
{
    ASTNode *tipo = parse_especificador_tipo(p);
    if (!tipo) return NULL;
    parse_asteriscos(p);
    if (p->current && p->current->type && strcmp(p->current->type, "IDENTIFIER") == 0) {
        const char *name = p->current->value ? p->current->value : "";
        int line = p->current->line;
        parser_next_token(p);
        parse_sufixo_array_opcional(p);
        if (p->current && p->current->value && strcmp(p->current->value, "=") == 0) {
            parser_next_token(p);
            parse_expressao(p);
        }
        while (p->current && p->current->value && strcmp(p->current->value, ",") == 0) {
            parser_next_token(p);
            parse_asteriscos(p);
            if (p->current && p->current->type && strcmp(p->current->type, "IDENTIFIER") == 0) {
                parser_next_token(p);
                parse_sufixo_array_opcional(p);
                if (p->current && p->current->value && strcmp(p->current->value, "=") == 0) { parser_next_token(p); parse_expressao(p); }
            } else break;
        }
        
        // Verificação explícita do ';' com emissão de erro e sincronização integrada
        if (p->current && p->current->value && strcmp(p->current->value, ";") == 0) {
            parser_next_token(p);
        } else {
            printf("Erro Sintático na linha %d: Esperado ';' após declaração da variável '%s'.\n", line, name);
            parser_sincronizar(p);
        }

        ASTNode *n = make_node(NODE_DECLARACAO_VARIAVEL, line);
        n->valor = strdup(name);
        add_filho(n, tipo);
        return n;
    }
    return NULL;
}

ASTNode *parse_instrucao_expressao(Parser *p)
{
    if (!p->current) return NULL;

    ASTNode *expr = parse_expressao(p);
    
    if (!expr) {
        printf("Erro Sintático na linha %d: Expressão inválida.\n", p->current ? p->current->line : 0);
        parser_sincronizar(p);
        return NULL;
    }

    if (p->current && p->current->value && strcmp(p->current->value, ";") == 0) {
        parser_next_token(p); 
    } else {
        printf("Erro Sintático na linha %d: Esperado ';' após a expressão.\n", expr->linha);
        // Não engolimos palavras-chave estruturais como o IF/WHILE/RETURN na sincronização
        if (p->current && p->current->type && 
            (strcmp(p->current->type, "KEYWORD_IF") == 0 || 
             strcmp(p->current->type, "KEYWORD_RETURN") == 0)) {
            // Deixa passar para o loop principal tratar a instrução seguinte
        } else {
            parser_sincronizar(p);
        }
    }

    ASTNode *node = make_node(NODE_INSTR_EXPR, expr->linha);
    add_filho(node, expr);
    return node;
}

ASTNode *parse_instrucao_if(Parser *p)
{
    if (!p->current || !(p->current->type && strcmp(p->current->type, "KEYWORD_IF") == 0)) return NULL;
    parser_next_token(p);
    parser_consume_if_value(p, "(");
    ASTNode *cond = parse_expressao(p);
    parser_consume_if_value(p, ")");
    ASTNode *then = parse_instrucao(p);
    ASTNode *n = make_node(NODE_IF, 0);
    if (cond) add_filho(n, cond);
    if (then) add_filho(n, then);
    if (p->current && p->current->type && strcmp(p->current->type, "KEYWORD_ELSE") == 0) {
        parser_next_token(p);
        ASTNode *els = parse_instrucao(p);
        if (els) add_filho(n, els);
    }
    return n;
}

ASTNode *parse_instrucao_while(Parser *p)
{
    if (!p->current || !(p->current->type && strcmp(p->current->type, "KEYWORD_WHILE") == 0)) return NULL;
    parser_next_token(p);
    parser_consume_if_value(p, "(");
    ASTNode *cond = parse_expressao(p);
    parser_consume_if_value(p, ")");
    ASTNode *body = parse_instrucao(p);
    ASTNode *n = make_node(NODE_WHILE, 0);
    if (cond) add_filho(n, cond);
    if (body) add_filho(n, body);
    return n;
}

ASTNode *parse_instrucao_for(Parser *p)
{
    if (!p->current || !(p->current->type && strcmp(p->current->type, "KEYWORD_FOR") == 0)) return NULL;
    parser_next_token(p);
    parser_consume_if_value(p, "(");
    // init
    ASTNode *init = NULL;
    if (p->current && !(p->current->value && strcmp(p->current->value, ";") == 0)) init = parse_expressao(p);
    parser_consume_if_value(p, ";");
    ASTNode *cond = NULL;
    if (p->current && !(p->current->value && strcmp(p->current->value, ";") == 0)) cond = parse_expressao(p);
    parser_consume_if_value(p, ";");
    ASTNode *inc = NULL;
    if (p->current && !(p->current->value && strcmp(p->current->value, ")") == 0)) inc = parse_expressao(p);
    parser_consume_if_value(p, ")");
    ASTNode *body = parse_instrucao(p);
    ASTNode *n = make_node(NODE_FOR, 0);
    if (init) add_filho(n, init);
    if (cond) add_filho(n, cond);
    if (inc) add_filho(n, inc);
    if (body) add_filho(n, body);
    return n;
}

ASTNode *parse_instrucao_do(Parser *p)
{
    if (!p->current || !(p->current->type && strcmp(p->current->type, "KEYWORD_DO") == 0)) return NULL;
    parser_next_token(p);
    ASTNode *body = parse_instrucao(p);
    parser_consume_if_value(p, "(");
    ASTNode *cond = parse_expressao(p);
    parser_consume_if_value(p, ")");
    parser_consume_if_value(p, ";");
    ASTNode *n = make_node(NODE_DO_WHILE, 0);
    if (body) add_filho(n, body);
    if (cond) add_filho(n, cond);
    return n;
}

ASTNode *parse_instrucao_return(Parser *p)
{
    if (!p->current || !(p->current->type && strcmp(p->current->type, "KEYWORD_RETURN") == 0)) return NULL;
    int line = p->current->line;
    parser_next_token(p);
    ASTNode *expr = NULL;
    if (p->current && !(p->current->value && strcmp(p->current->value, ";") == 0)) expr = parse_expressao(p);
    parser_consume_if_value(p, ";");
    ASTNode *n = make_node(NODE_RETURN, line);
    if (expr) add_filho(n, expr);
    return n;
}

ASTNode *parse_instrucao(Parser *p)
{
    // bloco
    if (p->current && p->current->value && strcmp(p->current->value, "{") == 0) return parse_bloco(p);
    // if
    if (p->current && p->current->type && strcmp(p->current->type, "KEYWORD_IF") == 0) return parse_instrucao_if(p);
    if (p->current && p->current->type && strcmp(p->current->type, "KEYWORD_WHILE") == 0) return parse_instrucao_while(p);
    if (p->current && p->current->type && strcmp(p->current->type, "KEYWORD_FOR") == 0) return parse_instrucao_for(p);
    if (p->current && p->current->type && strcmp(p->current->type, "KEYWORD_DO") == 0) return parse_instrucao_do(p);
    if (p->current && p->current->type && strcmp(p->current->type, "KEYWORD_RETURN") == 0) return parse_instrucao_return(p);
    // expression or empty ;
    if (p->current && p->current->value && strcmp(p->current->value, ";") == 0) { parser_next_token(p); return make_node(NODE_INSTR_EXPR, 0); }
    return parse_instrucao_expressao(p);
}
