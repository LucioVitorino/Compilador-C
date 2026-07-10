#ifndef AST_H
#define AST_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef enum {
    NODE_PROGRAMA,
    NODE_LISTA_DECL_GLOBAIS,
    NODE_DIRETIVA_INCLUDE,
    NODE_DIRETIVA_DEFINE,
    NODE_DECLARACAO_TYPEDEF,
    NODE_DECLARACAO_VARIAVEL,
    NODE_DECLARACAO_FUNCAO,
    NODE_BLOCO,
    NODE_IF,
    NODE_WHILE,
    NODE_DO_WHILE,
    NODE_FOR,
    NODE_SWITCH,
    NODE_CASE,
    NODE_RETURN,
    NODE_BREAK,
    NODE_CONTINUE,
    NODE_INSTR_EXPR,
    NODE_ATRIBUICAO,
    NODE_OP_BINARIO,
    NODE_OP_UNARIO,
    NODE_POS_FIXO,
    NODE_CHAMADA_FUNC,
    NODE_ACESSO_ARRAY,
    NODE_ACESSO_MEMBRO,
    NODE_IDENTIFICADOR,
    NODE_ESPECIFICADOR_TIPO,
    NODE_NOME_FICHEIRO,
    NODE_LIT_INTEIRO,
    NODE_LIT_REAL,
    NODE_LIT_CHAR,
    NODE_LIT_STRING,
} NodeType;

typedef struct ASTNode {
    NodeType type;
    char *valor;
    char *op;
    int linha;
    struct ASTNode **filhos;
    int n_filhos;
    int cap_filhos;
} ASTNode;

ASTNode *make_node(NodeType type, int linha);
ASTNode *make_folha(NodeType type, const char *valor, int linha);
void add_filho(ASTNode *pai, ASTNode *filho);
void print_ast(ASTNode *n, int depth);
void free_ast(ASTNode *n);

#endif
