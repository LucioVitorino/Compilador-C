#ifndef TABELA_SEMANTICO_H
#define TABELA_SEMANTICO_H

#include <stdbool.h>
#include "ast/ast.h"
#include <stdio.h>           // <=== FALTAVA (para o printf)
#include <stdlib.h>          // <=== FALTAVA (para o malloc e free)
#include <string.h>          // <=== FALTAVA (para o strcmp e strdup)


// 1. Enums para categorizar os dados (conforme a tua imagem)
typedef enum {
    CAT_VARIAVEL,
    CAT_METODO,     // Para funções
    CAT_CLASSE,
    CAT_PARAMETRO
} CategoriaSimbolo;

typedef enum {
    TIPO_INT,
    TIPO_FLOAT,
    TIPO_CHAR,
    TIPO_VOID,
    TIPO_STRING,
    TIPO_STRUCT
} TipoDado;

typedef enum {
    VAR_LOCAL,
    VAR_GLOBAL,
    VAR_ATRIBUTO
} TipoVariavel;

typedef enum {
    ESCOPO_GLOBAL,
    ESCOPO_CLASSE,
    ESCOPO_METODO
} EscopoSimbolo;

// Estrutura para os parâmetros de funções
typedef struct ParamNode {
    TipoDado tipo;
    char *nome;
    struct ParamNode *next;
} ParamNode;

// 2. A struct com TODOS os campos da especificação
typedef struct Symbol {
    char             *token;          // Tipo do token (ex: "IDENTIFIER")
    char             *lexeme;         // Nome real (ex: "x", "main")
    int              linha;           // Linha da declaração
    CategoriaSimbolo categoria;       // variável, método, etc.
    TipoDado         tipo_dado;       // int, float, char
    TipoVariavel     tipo_var;        // local, global
    EscopoSimbolo    escopo;          // global, método
    char             *valor;          // Valor inicial (guardado como string)
    long             endereco;        // Endereço simulado
    int              tamanho;         // Tamanho em bytes (ex: int = 4)
    bool             inicializado;    // true se foi inicializada (ex: x = 5)
    int              qtd_dimensoes;   // Arrays (0 para simples)
    ParamNode        *parametros;     // Lista de parâmetros se for função
    char             *modificadores;  // public, private, static
    TipoDado         retorno;         // Tipo de retorno se for função
    char             *tipo_customizado; // Nome de struct/typedef customizado
    struct SymbolTable *escopo_membros; // Tabela de símbolos para atributos de struct
    
    struct Symbol    *next;           // Próximo em caso de colisão na Hash
} Symbol;

#define HASH_SIZE 211

// Estrutura da Tabela
typedef struct SymbolTable {
    Symbol              *table[HASH_SIZE];
    struct SymbolTable  *parent;      // Aponta para o escopo pai (Local -> Global)
    long                 next_endereco; // Endereçamento simulado para o escopo actual
} SymbolTable;

// Protótipos das funções que já tens implementadas
SymbolTable  *create_table(SymbolTable *parent);
unsigned int hash_function(const char *lexeme);
bool         insert_symbol(SymbolTable *st, Symbol *sym);
Symbol       *lookup_symbol(SymbolTable *st, const char *lexeme);
Symbol       *lookup_symbol_current_scope(SymbolTable *st, const char *lexeme);
const char   *tipo_dado_to_string(TipoDado tipo);
bool         tipo_dado_eh_numerico(TipoDado tipo);
bool         tipos_compativeis(TipoDado destino, TipoDado origem);
void         free_table(SymbolTable *st);
void         print_table(SymbolTable *st);
void         analisar_semantica(ASTNode *node, SymbolTable *st, EscopoSimbolo escopo_atual);
int          obter_numero_erros_semanticos(void);
TipoDado     obter_tipo_expressao(ASTNode *node, SymbolTable *st);

#endif