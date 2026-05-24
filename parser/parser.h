#ifndef PARSER_H
#define PARSER_H

#include "../ast/ast.h"
#include "../Lexer/token.h"

typedef struct Parser {
    t_token *token_stream; /* pointer to tokens list head */
    t_token *current; /* current token pointer */
} Parser;

void parser_init(Parser *p, t_token *tokens);
ASTNode *parse_programa(Parser *p);
void parser_free(Parser *p);

/* internal token navigation (exposed across parser modules) */
t_token *parser_next_token(Parser *p);

/* expression parsing */
ASTNode *parse_expressao(Parser *p);
ASTNode *parse_aditivo(Parser *p);
ASTNode *parse_multiplicativo(Parser *p);
ASTNode *parse_primario(Parser *p);
ASTNode *parse_unario(Parser *p);
ASTNode *parse_pos_fixo(Parser *p);
ASTNode *parse_atribuicao(Parser *p);
ASTNode *parse_logico_ou(Parser *p);
ASTNode *parse_logico_e(Parser *p);
ASTNode *parse_igualdade(Parser *p);
ASTNode *parse_relacional(Parser *p);
ASTNode *parse_argumentos_opcionais(Parser *p);

/* top-level declarations */
ASTNode *parse_lista_decl_globais(Parser *p);
ASTNode *parse_declaracao_global(Parser *p);
ASTNode *parse_especificador_tipo(Parser *p);
/* statements and blocks */
ASTNode *parse_bloco(Parser *p);
ASTNode *parse_lista_itens_bloco(Parser *p);
ASTNode *parse_item_bloco(Parser *p);
ASTNode *parse_declaracao_variavel_local(Parser *p);
ASTNode *parse_instrucao(Parser *p);
ASTNode *parse_instrucao_expressao(Parser *p);
ASTNode *parse_instrucao_if(Parser *p);
ASTNode *parse_instrucao_while(Parser *p);
ASTNode *parse_instrucao_for(Parser *p);
ASTNode *parse_instrucao_do(Parser *p);
ASTNode *parse_instrucao_return(Parser *p);

/* small helpers (implemented in helpers.c) */
int parse_asteriscos(Parser *p);
int parse_sufixo_array_opcional(Parser *p);
ASTNode *parse_parametros_opcionais(Parser *p);

/* utilities */
const char *parser_peek_value(Parser *p);
const char *parser_peek_type(Parser *p);
int parser_consume_if_value(Parser *p, const char *val);
int parser_consume_if_type(Parser *p, const char *type);
void parser_sincronizar(Parser *p);


#endif
