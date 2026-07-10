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

ASTNode *parse_diretiva_include(Parser *p)
{
    if (!p->current || strcmp(p->current->type, "TOK_HASH") != 0)
        return NULL;
        
    int line = p->current->line;
    parser_next_token(p); // Consome '#'
    
    if (!parser_expect(p, "include", "Esperado 'include' após '#'", SYNC_INSTRUCAO)) {
        recover(p, SYNC_INSTRUCAO); // Salta para a próxima linha/instrução segura
        return NULL;
    }
    parser_next_token(p); // Consome 'include'
    
    // 1. Validar a abertura do delimitador '<'
    if (!parser_expect(p, "<", "Falta o delimitador de abertura '<' na diretiva #include", SYNC_INSTRUCAO)) {
        recover(p, SYNC_INSTRUCAO);
        return NULL;
    }
    parser_next_token(p); // Consome '<'
    
    // 2. Validar o nome do ficheiro
    if (!p->current || strcmp(p->current->type, "TOK_NOME_FICHEIRO") != 0) {
        syntax_error_recover(p, "Nome de ficheiro inválido na diretiva #include", SYNC_INSTRUCAO);
        return NULL;
    }
    char *filename = strdup(p->current->value);
    parser_next_token(p); // Consome o nome do ficheiro
    
    // 3. Validar o fecho do delimitador '>'
    if (!parser_expect(p, ">", "Falta o delimitador de fecho '>' na diretiva #include", SYNC_INSTRUCAO)) {
        recover(p, SYNC_INSTRUCAO);
        free(filename);
        return NULL;
    }
    parser_next_token(p); // Consome '>'
    
    ASTNode *inc = make_node(NODE_DIRETIVA_INCLUDE, line);
    ASTNode *file = make_folha(NODE_NOME_FICHEIRO, filename, line);
    add_filho(inc, file);
    
    free(filename);
    return inc;
}

ASTNode *parse_programa(Parser *p)
{
    // Cria o nó raiz da árvore sintática
    ASTNode *programa = make_node(NODE_PROGRAMA, 0);
    ASTNode *lista_globais = make_node(NODE_LISTA_DECL_GLOBAIS, 0);
    add_filho(programa, lista_globais);

    // Loop principal até chegar ao fim do ficheiro (EOF)
    while (p->current && strcmp(p->current->type, "TOK_EOF") != 0) {
        
        // Guardamos o token atual para blindagem absoluta anti-loop
        t_token *antes_do_erro = p->current;
        
        ASTNode *decl_global = parse_declaracao_global(p);
        
        if (decl_global) {
            add_filho(lista_globais, decl_global);
        } else {
            if (p->current == antes_do_erro) {
                parser_next_token(p);
            }
        }
    }

    return programa;
}