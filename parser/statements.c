#include "parser.h"

ASTNode *parse_bloco(Parser *p)
{
    if (!p->current || !(p->current->value && strcmp(p->current->value, "{") == 0)) 
        return NULL;
        
    int line = p->current->line;
    parser_next_token(p); // Consome '{'
    
    ASTNode *blk = make_node(NODE_BLOCO, line);
    ASTNode *list = parse_lista_itens_bloco(p);
    if (list) add_filho(blk, list);
    
    // Garante '}' universalmente para fechar blocos de funções/estruturas
    parser_expect(p, "}", "Falta a chaveta de fecho '}' para fechar o bloco");
    
    return blk;
}

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
            parser_sincronizar_geral(p); // Sincronização Geral Robustecida
        }
    }
    return root;
}

ASTNode *parse_item_bloco(Parser *p)
{
    if (p->current && p->current->type && 
        (strcmp(p->current->type, "KEYWORD_INT") == 0 || 
         strcmp(p->current->type, "KEYWORD_FLOAT") == 0 || 
         strcmp(p->current->type, "KEYWORD_CHAR") == 0 || 
         strcmp(p->current->type, "KEYWORD_VOID") == 0 || 
         strcmp(p->current->type, "IDENTIFIER") == 0)) {
        
        ASTNode *saved = parse_declaracao_variavel_local(p);
        if (saved) return saved;
    }
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
                if (p->current && p->current->value && strcmp(p->current->value, "=") == 0) { 
                    parser_next_token(p); 
                    parse_expressao(p); 
                }
            } else break;
        }
        
        // Garante o ponto e vírgula de forma centralizada
        parser_expect(p, ";", "Esperado ';' após declaração de variável");

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
    
    // Captura a linha real de início ANTES de processar a expressão para evitar L0
    int linha_atual = p->current->line; 

    ASTNode *expr = parse_expressao(p);
    
    if (!expr) {
        printf("Erro Sintático na linha %d: Expressão inválida.\n", linha_atual);
        parser_sincronizar_geral(p);
        return NULL;
    }

    if (p->current && p->current->value && strcmp(p->current->value, ";") == 0) {
        parser_next_token(p); 
    } else {
        printf("Erro Sintático na linha %d: Esperado ';' após a expressão.\n", linha_atual);
        if (p->current && p->current->type && 
            (strcmp(p->current->type, "KEYWORD_IF") == 0 || 
             strcmp(p->current->type, "KEYWORD_RETURN") == 0)) {
            // Deixa passar sem consumir para o loop principal tratar a palavra-chave
        } else {
            parser_sincronizar_geral(p); 
        }
    }

    ASTNode *node = make_node(NODE_INSTR_EXPR, linha_atual);
    add_filho(node, expr);
    return node;
}

ASTNode *parse_instrucao_if(Parser *p)
{
    if (!p->current || !(p->current->type && strcmp(p->current->type, "KEYWORD_IF") == 0)) 
        return NULL;
        
    int line = p->current->line;
    parser_next_token(p); // Consome 'if'

    // Garante '(' com mensagem limpa
    if (!parser_expect(p, "(", "Falta o parêntese de abertura após o 'if'"))
        return make_node(NODE_IF, line);

    ASTNode *cond = parse_expressao(p);

    // Garante ')' com pânico controlado se faltar
    parser_expect(p, ")", "Falta o parêntese de fecho da condição do 'if'");

    ASTNode *body = parse_instrucao(p);

    ASTNode *n = make_node(NODE_IF, line);
    if (cond) add_filho(n, cond);
    if (body) add_filho(n, body);
    return n;
}

ASTNode *parse_instrucao_while(Parser *p)
{
    if (!p->current || !(p->current->type && strcmp(p->current->type, "KEYWORD_WHILE") == 0)) 
        return NULL;
        
    int line = p->current->line;
    parser_next_token(p); // Consome 'while'
    
    if (!parser_expect(p, "(", "Falta o parêntese de abertura no loop 'while'"))
        return make_node(NODE_WHILE, line);
        
    ASTNode *cond = parse_expressao(p);
    
    parser_expect(p, ")", "Falta o parêntese de fecho da condição do 'while'");
    
    ASTNode *body = parse_instrucao(p);
    
    ASTNode *n = make_node(NODE_WHILE, line);
    if (cond) add_filho(n, cond);
    if (body) add_filho(n, body);
    return n;
}

ASTNode *parse_instrucao_for(Parser *p)
{
    if (!p->current || !(p->current->type && strcmp(p->current->type, "KEYWORD_FOR") == 0)) 
        return NULL;
        
    int line = p->current->line;
    parser_next_token(p); // Consome 'for'
    
    if (!parser_expect(p, "(", "Falta o parêntese de abertura no loop 'for'"))
        return make_node(NODE_FOR, line);
        
    ASTNode *init = NULL;
    if (p->current && !(p->current->value && strcmp(p->current->value, ";") == 0)) 
        init = parse_expressao(p);
    parser_expect(p, ";", "Esperado ';' após inicialização do loop 'for'");
    
    ASTNode *cond = NULL;
    if (p->current && !(p->current->value && strcmp(p->current->value, ";") == 0)) 
        cond = parse_expressao(p);
    parser_expect(p, ";", "Esperado ';' após condição de paragem do loop 'for'");
    
    ASTNode *inc = NULL;
    if (p->current && !(p->current->value && strcmp(p->current->value, ")") == 0)) 
        inc = parse_expressao(p);
    parser_expect(p, ")", "Falta o parêntese de fecho do cabeçalho do 'for'");
    
    ASTNode *body = parse_instrucao(p);
    
    ASTNode *n = make_node(NODE_FOR, line);
    if (init) add_filho(n, init);
    if (cond) add_filho(n, cond);
    if (inc) add_filho(n, inc);
    if (body) add_filho(n, body);
    return n;
}

ASTNode *parse_instrucao_do(Parser *p)
{
    if (!p->current || !(p->current->type && strcmp(p->current->type, "KEYWORD_DO") == 0)) 
        return NULL;
        
    int line = p->current->line;
    parser_next_token(p); // Consome 'do'
    
    ASTNode *body = parse_instrucao(p);
    
    parser_expect(p, "while", "Esperada a palavra-chave 'while' após bloco 'do'");
    parser_expect(p, "(", "Falta o parêntese de abertura na condição do 'do-while'");
    
    ASTNode *cond = parse_expressao(p);
    
    parser_expect(p, ")", "Falta o parêntese de fecho da condição do 'do-while'");
    parser_expect(p, ";", "Esperado ';' no término do comando 'do-while'");
    
    ASTNode *n = make_node(NODE_DO_WHILE, line);
    if (body) add_filho(n, body);
    if (cond) add_filho(n, cond);
    return n;
}

ASTNode *parse_instrucao_return(Parser *p)
{
    if (!p->current || !(p->current->type && strcmp(p->current->type, "KEYWORD_RETURN") == 0)) 
        return NULL;
        
    int line = p->current->line;
    parser_next_token(p); // Consome 'return'
    
    ASTNode *expr = NULL;
    if (p->current && !(p->current->value && strcmp(p->current->value, ";") == 0)) {
        expr = parse_expressao(p);
    }
    
    parser_expect(p, ";", "Falta o ponto e vírgula no final do comando return");

    ASTNode *n = make_node(NODE_RETURN, line);
    if (expr) add_filho(n, expr);
    return n;
}

ASTNode *parse_instrucao(Parser *p)
{
    if (p->current && p->current->value && strcmp(p->current->value, "{") == 0) return parse_bloco(p);
    if (p->current && p->current->type && strcmp(p->current->type, "KEYWORD_IF") == 0) return parse_instrucao_if(p);
    if (p->current && p->current->type && strcmp(p->current->type, "KEYWORD_WHILE") == 0) return parse_instrucao_while(p);
    if (p->current && p->current->type && strcmp(p->current->type, "KEYWORD_FOR") == 0) return parse_instrucao_for(p);
    if (p->current && p->current->type && strcmp(p->current->type, "KEYWORD_DO") == 0) return parse_instrucao_do(p);
    if (p->current && p->current->type && strcmp(p->current->type, "KEYWORD_RETURN") == 0) return parse_instrucao_return(p);
    
    if (p->current && p->current->value && strcmp(p->current->value, ";") == 0) { 
        parser_next_token(p); 
        return make_node(NODE_INSTR_EXPR, 0); 
    }
    return parse_instrucao_expressao(p);
}