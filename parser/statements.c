#include "parser.h"
#include <string.h>
#include <stdlib.h>

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
    parser_expect(p, "}", "Falta a chaveta de fecho '}' para fechar o bloco", SYNC_BLOCO);
    
    return blk;
}

ASTNode *parse_lista_itens_bloco(Parser *p)
{
    ASTNode *root = make_node(NODE_LISTA_DECL_GLOBAIS, 0);

    while (p->current && p->current->value && strcmp(p->current->value, "}") != 0 && strcmp(p->current->type, "TOK_EOF") != 0) {
        
        // Pular múltiplos comentários seguidos ou intercalados
        if (p->current->type && strcmp(p->current->type, "COMMENT") == 0) {
            parser_next_token(p);
            continue;
        }

        ASTNode *it = parse_item_bloco(p);
        if (it) {
            add_filho(root, it);
        } else {
            // Só entra em pânico se o token REAL não for uma instrução válida
            syntax_error_recover(p, "Instrução ou item de bloco inválido.", SYNC_INSTRUCAO);
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
    
    if (!p->current || !p->current->type || strcmp(p->current->type, "IDENTIFIER") != 0) {
        syntax_error_recover(p, "Esperado identificador após o especificador de tipo", SYNC_INSTRUCAO);
        return NULL;
    }

    const char *name = p->current->value ? p->current->value : "";
    int line = p->current->line;
    parser_next_token(p);
    parse_sufixo_array_opcional(p);
    
    // Criação do nó principal da declaração
    ASTNode *n = make_node(NODE_DECLARACAO_VARIAVEL, line);
    n->valor = strdup(name);
    add_filho(n, tipo); // Tipo é o primeiro filho

    // Captura da inicialização da primeira variável
    if (p->current && p->current->value && strcmp(p->current->value, "=") == 0) {
        parser_next_token(p); // Consome '='
        ASTNode *expr = parse_expressao(p);
        if (expr) {
            add_filho(n, expr); // O valor literal/expressão passa a ser o filho do nó
        } else {
            syntax_error_recover(p, "Expressão de inicialização inválida", SYNC_INSTRUCAO);
        }
    }

    // Suporte para declarações múltiplas na mesma linha (ex: int x = 5, y = 10;)
    while (p->current && p->current->value && strcmp(p->current->value, ",") == 0) {
        parser_next_token(p); // Consome ','
        parse_asteriscos(p);
        
        if (p->current && p->current->type && strcmp(p->current->type, "IDENTIFIER") == 0) {
            const char *next_name = p->current->value ? p->current->value : "";
            int next_line = p->current->line;
            parser_next_token(p);
            parse_sufixo_array_opcional(p);
            
            ASTNode *next_var = make_node(NODE_DECLARACAO_VARIAVEL, next_line);
            next_var->valor = strdup(next_name);
            
            ASTNode *tipo_clone = make_node(NODE_ESPECIFICADOR_TIPO, next_line);
            tipo_clone->valor = strdup(tipo->valor ? tipo->valor : "int");
            add_filho(next_var, tipo_clone);

            if (p->current && p->current->value && strcmp(p->current->value, "=") == 0) { 
                parser_next_token(p); // Consome '='
                ASTNode *next_expr = parse_expressao(p);
                if (next_expr) add_filho(next_var, next_expr);
            }
            add_filho(n, next_var); 
        } else {
            syntax_error_recover(p, "Esperado identificador após ',' na declaração", SYNC_INSTRUCAO);
            break;
        }
    }
    
    // Garante o ponto e vírgula de forma centralizada utilizando o sync de instruções locais
    parser_expect(p, ";", "Esperado ';' após declaração de variável", SYNC_INSTRUCAO);

    return n;
}

ASTNode *parse_instrucao_expressao(Parser *p)
{
    if (!p->current) return NULL;
    
    int linha_atual = p->current->line; 
    ASTNode *expr = parse_expressao(p);
    
    if (!expr) {
        syntax_error_recover(p, "Expressão inválida.", SYNC_EXPRESSAO);
        return NULL;
    }

    if (p->current && p->current->value && strcmp(p->current->value, ";") == 0) {
        parser_next_token(p); 
    } else {
        if (p->current && p->current->type && 
            (strcmp(p->current->type, "KEYWORD_IF") == 0 || 
             strcmp(p->current->type, "KEYWORD_RETURN") == 0)) {
            // Deixa passar sem consumir para o loop principal tratar a palavra-chave
        } else {
            syntax_error_recover(p, "Esperado ';' após a expressão.", SYNC_INSTRUCAO);
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

    // Se falhar o '(', recupera até à condição ou ao fecho do cabeçalho
    if (!parser_expect(p, "(", "Falta o parêntese de abertura após o 'if'", SYNC_HEADER)) {
        // Se falhou gravemente, tenta avançar até ao corpo
        recover(p, SYNC_HEADER);
    }

    ASTNode *cond = parse_expressao(p);

    // Se faltar o ')', reporta o erro, mas sincroniza no SYNC_HEADER para ler o corpo!
    if (!parser_expect(p, ")", "Falta o parêntese de fecho da condição do 'if'", SYNC_HEADER)) {
        if (p->current && p->current->value && strcmp(p->current->value, ")") == 0) {
            parser_next_token(p);
        }
    }

    // Processa o corpo de forma segura
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
    
    if (!parser_expect(p, "(", "Falta o parêntese de abertura no loop 'while'", SYNC_HEADER)) {
        recover(p, SYNC_HEADER);
    }
        
    ASTNode *cond = parse_expressao(p);
    
    if (!parser_expect(p, ")", "Falta o parêntese de fecho da condição do 'while'", SYNC_HEADER)) {
        if (p->current && p->current->value && strcmp(p->current->value, ")") == 0) {
            parser_next_token(p);
        }
    }
    
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
    
    if (!parser_expect(p, "(", "Esperado '(' após a palavra-chave 'for'", SYNC_HEADER)) {
        recover(p, SYNC_HEADER);
    }
        
    // 1. Inicialização
    ASTNode *init = NULL;
    if (p->current && !(p->current->value && strcmp(p->current->value, ";") == 0)) 
        init = parse_expressao(p);
    
    if (!parser_expect(p, ";", "Esperado ';' após a inicialização do 'for'", SYNC_HEADER)) {
        recover(p, SYNC_HEADER);
    }
    
    // 2. Condição de paragem
    ASTNode *cond = NULL;
    if (p->current && !(p->current->value && strcmp(p->current->value, ";") == 0)) 
        cond = parse_expressao(p);
        
    if (!parser_expect(p, ";", "Esperado ';' após a condição do 'for'", SYNC_HEADER)) {
        // Se falhar o ';', o pânico consome até encontrar o fecho ')' ou o início do bloco '{'
        recover(p, SYNC_HEADER);
    }
    
    // 3. Incremento
    ASTNode *inc = NULL;
    if (p->current && !(p->current->value && strcmp(p->current->value, ")") == 0)) 
        inc = parse_expressao(p);
        
    if (!parser_expect(p, ")", "Esperado ')' após o cabeçalho do 'for'", SYNC_HEADER)) {
        if (p->current && p->current->value && strcmp(p->current->value, ")") == 0) {
            parser_next_token(p);
        }
    }
    
    // 4. Corpo do For
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
    
    // 1. Processa o corpo (geralmente o bloco '{ ... }')
    ASTNode *body = parse_instrucao(p);
    if (!body) {
        syntax_error_recover(p, "Corpo do 'do-while' inválido ou vazio", SYNC_HEADER);
    }
    
    // === CORREÇÃO CRÍTICA AQUI ===
    // Validar e consumir explicitamente o 'while' do fim do laço
    if (p->current && p->current->type && strcmp(p->current->type, "KEYWORD_WHILE") == 0) {
        parser_next_token(p); // Consome com sucesso o 'while' do do-while
    } else {
        syntax_error_recover(p, "Esperado 'while' após o corpo do comando 'do'", SYNC_HEADER);
    }
    
    // 2. Garante o parêntese de abertura '('
    if (!parser_expect(p, "(", "Esperado '(' após a palavra-chave 'while' no 'do-while'", SYNC_HEADER)) {
        recover(p, SYNC_HEADER);
    }
    
    // 3. Processa a condição perfeitamente
    ASTNode *cond = parse_expressao(p);
    
    // 4. Garante o parêntese de fecho ')'
    if (!parser_expect(p, ")", "Esperado ')' após a condição do 'do-while'", SYNC_HEADER)) {
        if (p->current && p->current->value && strcmp(p->current->value, ")") == 0) {
            parser_next_token(p);
        }
    }
    
    // 5. Garante o ';' final obrigatório
    if (!parser_expect(p, ";", "Falta o ';' no final da instrução do-while", SYNC_INSTRUCAO)) {
        // Se não houver ';', usamos o SYNC_INSTRUCAO para alinhar na próxima linha
        recover(p, SYNC_INSTRUCAO);
    }
    
    // Construção segura da AST
    ASTNode *n = make_node(NODE_WHILE, line); // Ou NODE_DO_WHILE
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
    
    parser_expect(p, ";", "Falta o ponto e vírgula no final do comando return", SYNC_INSTRUCAO);

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