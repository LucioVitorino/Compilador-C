#include "parser.h"

ASTNode *parse_primario(Parser *p)
{
    if (!p->current) return NULL;
    if (p->current->type && (strcmp(p->current->type, "INT_LITERAL") == 0 || strcmp(p->current->type, "FLOAT_LITERAL") == 0 || strcmp(p->current->type, "IDENTIFIER") == 0 || strcmp(p->current->type, "STRING_LITERAL") == 0 || strcmp(p->current->type, "CHAR_LITERAL") == 0)) {
        const char *val = p->current->value ? p->current->value : "";
        int line = p->current->line;
        parser_next_token(p);
        return make_folha(NODE_IDENTIFICADOR, val, line);
    }
    if (parser_consume_if_value(p, "(")) {
        ASTNode *e = parse_expressao(p);
        if (!e) {
            // Se a expressão interna for completamente inválida (ex: '( * 5);')
            syntax_error_recover(p, "Expressão inválida ou vazia dentro dos parênteses", SYNC_EXPRESSAO);
            return NULL;
        }
        
        // Garante o parêntese de fecho ')'. Se não encontrar, entra em pânico controlado.
        if (!parser_expect(p, ")", "Falta o parêntese de fecho ')' na expressão", SYNC_EXPRESSAO)) {
            if (p->current && p->current->value && strcmp(p->current->value, ";") != 0) {
                parser_next_token(p); 
            }
        }
        return e;
    }
}

/* multiplicative and additive are implemented below with unary/posfix handling */

ASTNode *parse_expressao(Parser *p)
{
    return parse_atribuicao(p);
}

/* pos_fixo: primario pos_fixo_r */
ASTNode *parse_pos_fixo(Parser *p)
{
    ASTNode *n = parse_primario(p);
    if (!n) return NULL;
    while (p->current && p->current->value) {
        if (strcmp(p->current->value, "[") == 0) {
            parser_next_token(p);
            ASTNode *idx = parse_expressao(p);
            parser_consume_if_value(p, "]");
            ASTNode *acc = make_node(NODE_ACESSO_ARRAY, 0);
            add_filho(acc, n);
            add_filho(acc, idx);
            n = acc;
        } else if (strcmp(p->current->value, "(") == 0) {
            int line = p->current->line;
            parser_next_token(p); // Consome '('
            
            ASTNode *args = parse_argumentos_opcionais(p);
            
            if (!parser_expect(p, ")", "Falta o parêntese de fecho ')' na chamada da função", SYNC_EXPRESSAO)) {
                if (p->current && p->current->value && strcmp(p->current->value, ";") != 0) {
                    parser_next_token(p);
                }
            }
            
            ASTNode *call = make_node(NODE_CHAMADA_FUNC, line);
            add_filho(call, n);
            if (args) add_filho(call, args);
            n = call;
        } else if (strcmp(p->current->value, ".") == 0) {
            parser_next_token(p);
            if (p->current && p->current->type && strcmp(p->current->type, "IDENTIFIER") == 0) {
                ASTNode *m = make_node(NODE_ACESSO_MEMBRO, 0);
                m->valor = strdup(p->current->value);
                parser_next_token(p);
                add_filho(m, n);
                n = m;
            }
        } else if (strcmp(p->current->value, "++") == 0 || strcmp(p->current->value, "--") == 0) {
            char *op = strdup(p->current->value);
            parser_next_token(p);
            ASTNode *posfix = make_node(NODE_POS_FIXO, 0);
            posfix->op = op;
            add_filho(posfix, n);
            n = posfix;
        } else break;
    }
    return n;
}

ASTNode *parse_unario(Parser *p)
{
    if (!p->current) return NULL;
    if (p->current->value && strcmp(p->current->value, "!") == 0) {
        parser_next_token(p);
        ASTNode *sub = parse_unario(p);
        ASTNode *n = make_node(NODE_OP_UNARIO, 0);
        add_filho(n, sub);
        n->op = strdup("!");
        return n;
    }
    if (p->current->value && strcmp(p->current->value, "-") == 0) {
        parser_next_token(p);
        ASTNode *sub = parse_unario(p);
        ASTNode *n = make_node(NODE_OP_UNARIO, 0);
        add_filho(n, sub);
        n->op = strdup("-");
        return n;
    }
    if (p->current->value && strcmp(p->current->value, "&") == 0) {
        parser_next_token(p);
        ASTNode *sub = parse_unario(p);
        ASTNode *n = make_node(NODE_OP_UNARIO, 0);
        add_filho(n, sub);
        n->op = strdup("&");
        return n;
    }
    if (p->current->value && strcmp(p->current->value, "*") == 0) {
        parser_next_token(p);
        ASTNode *sub = parse_unario(p);
        ASTNode *n = make_node(NODE_OP_UNARIO, 0);
        add_filho(n, sub);
        n->op = strdup("*");
        return n;
    }
    return parse_pos_fixo(p);
}

ASTNode *parse_multiplicativo(Parser *p)
{
    ASTNode *left = parse_unario(p);
    while (p->current && p->current->value && (strcmp(p->current->value, "*") == 0 || strcmp(p->current->value, "/") == 0 || strcmp(p->current->value, "%") == 0)) {
        char *op = strdup(p->current->value);
        parser_next_token(p);
        ASTNode *right = parse_unario(p);
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

ASTNode *parse_relacional(Parser *p)
{
    ASTNode *left = parse_aditivo(p);
    while (p->current && p->current->value) {
        const char *v = p->current->value;
        if (strcmp(v, "<") == 0 || strcmp(v, ">") == 0 || strcmp(v, "<=") == 0 || strcmp(v, ">=") == 0) {
            char *op = strdup(v);
            parser_next_token(p);
            ASTNode *right = parse_aditivo(p);
            ASTNode *node = make_node(NODE_OP_BINARIO, 0);
            node->op = op;
            add_filho(node, left);
            add_filho(node, right);
            left = node;
        } else break;
    }
    return left;
}

ASTNode *parse_bit_shift(Parser *p)
{
    ASTNode *left = parse_relacional(p);
    while (p->current && p->current->value) {
        const char *v = p->current->value;
        if (strcmp(v, "<<") == 0 || strcmp(v, ">>") == 0) {
            char *op = strdup(v);
            parser_next_token(p);
            ASTNode *right = parse_relacional(p);
            ASTNode *node = make_node(NODE_OP_BINARIO, 0);
            node->op = op;
            add_filho(node, left);
            add_filho(node, right);
            left = node;
        } else break;
    }
    return left;
}

ASTNode *parse_igualdade(Parser *p)
{
    ASTNode *left = parse_bit_shift(p);
    while (p->current && p->current->value) {
        const char *v = p->current->value;
        if (strcmp(v, "==") == 0 || strcmp(v, "!=") == 0) {
            char *op = strdup(v);
            parser_next_token(p);
            ASTNode *right = parse_bit_shift(p);
            ASTNode *node = make_node(NODE_OP_BINARIO, 0);
            node->op = op;
            add_filho(node, left);
            add_filho(node, right);
            left = node;
        } else break;
    }
    return left;
}

ASTNode *parse_bit_e(Parser *p)
{
    ASTNode *left = parse_igualdade(p);
    while (p->current && p->current->value && strcmp(p->current->value, "&") == 0) {
        char *op = strdup(p->current->value);
        parser_next_token(p);
        ASTNode *right = parse_igualdade(p);
        ASTNode *node = make_node(NODE_OP_BINARIO, 0);
        node->op = op;
        add_filho(node, left);
        add_filho(node, right);
        left = node;
    }
    return left;
}

ASTNode *parse_bit_xor(Parser *p)
{
    ASTNode *left = parse_bit_e(p);
    while (p->current && p->current->value && strcmp(p->current->value, "^") == 0) {
        char *op = strdup(p->current->value);
        parser_next_token(p);
        ASTNode *right = parse_bit_e(p);
        ASTNode *node = make_node(NODE_OP_BINARIO, 0);
        node->op = op;
        add_filho(node, left);
        add_filho(node, right);
        left = node;
    }
    return left;
}

ASTNode *parse_bit_ou(Parser *p)
{
    ASTNode *left = parse_bit_xor(p);
    while (p->current && p->current->value && strcmp(p->current->value, "|") == 0) {
        char *op = strdup(p->current->value);
        parser_next_token(p);
        ASTNode *right = parse_bit_xor(p);
        ASTNode *node = make_node(NODE_OP_BINARIO, 0);
        node->op = op;
        add_filho(node, left);
        add_filho(node, right);
        left = node;
    }
    return left;
}

ASTNode *parse_logico_e(Parser *p)
{
    ASTNode *left = parse_bit_ou(p);
    while (p->current && p->current->value && strcmp(p->current->value, "&&") == 0) {
        char *op = strdup(p->current->value);
        parser_next_token(p);
        ASTNode *right = parse_bit_ou(p);
        ASTNode *node = make_node(NODE_OP_BINARIO, 0);
        node->op = op;
        add_filho(node, left);
        add_filho(node, right);
        left = node;
    }
    return left;
}

ASTNode *parse_logico_ou(Parser *p)
{
    ASTNode *left = parse_logico_e(p);
    while (p->current && p->current->value && strcmp(p->current->value, "||") == 0) {
        char *op = strdup(p->current->value);
        parser_next_token(p);
        ASTNode *right = parse_logico_e(p);
        ASTNode *node = make_node(NODE_OP_BINARIO, 0);
        node->op = op;
        add_filho(node, left);
        add_filho(node, right);
        left = node;
    }
    return left;
}

// Em expressao.c — Garantindo suporte a todos os operadores da nova gramática
ASTNode *parse_atribuicao(Parser *p)
{
    ASTNode *left = parse_logico_ou(p);
    
    if (p->current && p->current->value) {
        const char *v = p->current->value;
        // Agora todos os operadores vêm unificados do Lexer com precisão
        if (strcmp(v, "=") == 0 || strcmp(v, "+=") == 0 || strcmp(v, "-=") == 0 || 
            strcmp(v, "*=") == 0 || strcmp(v, "/=") == 0 || strcmp(v, "%=") == 0) {

            char *op = strdup(v);
            parser_next_token(p); // Consome o operador unificado de atribuição

            ASTNode *right = parse_atribuicao(p); // Associatividade correta à direita
            ASTNode *node = make_node(NODE_ATRIBUICAO, left->linha);
            node->op = op;
            add_filho(node, left);
            add_filho(node, right);
            return node;
        }
    }
    return left;
}

ASTNode *parse_argumentos_opcionais(Parser *p)
{
    if (!p->current) return NULL;
    // Se a chamada for imediata (ex: funcao()), não há argumentos
    if (p->current->value && strcmp(p->current->value, ")") == 0) return NULL;
    
    ASTNode *root = make_node(NODE_LISTA_DECL_GLOBAIS, p->current->line);
    
    // 1. Processa o primeiro argumento
    ASTNode *first = parse_expressao(p);
    if (!first) {
        syntax_error_recover(p, "Argumento inválido ou vazio na chamada da função", SYNC_EXPRESSAO);
    } else {
        add_filho(root, first);
    }
    
    // 2. Processa os restantes argumentos separados por vírgula
    while (p->current && p->current->value && strcmp(p->current->value, ",") == 0) {
        parser_next_token(p); // Consome ','
        
        // Proteção contra vírgula órfã no fim: funcao(a, b, )
        if (p->current && p->current->value && strcmp(p->current->value, ")") == 0) {
            syntax_error_recover(p, "Esperado argumento após a vírgula", SYNC_EXPRESSAO);
            break;
        }
        
        ASTNode *e = parse_expressao(p);
        if (!e) {
            syntax_error_recover(p, "Argumento inválido após a vírgula", SYNC_EXPRESSAO);
        } else {
            add_filho(root, e);
        }
    }
    
    return root;
}
