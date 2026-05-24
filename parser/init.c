#include "parser.h"

t_token *parser_next_token(Parser *p)
{
    if (!p->current) return NULL;
    t_token *tok = p->current;
    p->current = p->current->next;
    return tok;
}

const char *parser_peek_value(Parser *p) { return p->current ? p->current->value : NULL; }
const char *parser_peek_type(Parser *p) { return p->current ? p->current->type : NULL; }
int parser_consume_if_value(Parser *p, const char *val) {
    if (p->current && p->current->value && strcmp(p->current->value, val) == 0) { parser_next_token(p); return 1; }
    return 0;
}
int parser_consume_if_type(Parser *p, const char *type) {
    if (p->current && p->current->type && strcmp(p->current->type, type) == 0) { parser_next_token(p); return 1; }
    return 0;
}

void parser_init(Parser *p, t_token *tokens)
{
    p->token_stream = tokens;
    p->current = tokens;
}

void parser_free(Parser *p)
{
    (void)p;
}

void parser_sincronizar_geral(Parser *p)
{
    if (!p->current) return;
    int linha_erro = p->current->line;

    while (p->current)
    {
        if (p->current->type && strcmp(p->current->type, "TOK_EOF") == 0) break;
        
        // Se avançar para a linha seguinte, para o pânico imediatamente
        if (p->current->line > linha_erro) break; 

        if (p->current->value && strcmp(p->current->value, ";") == 0) {
            parser_next_token(p); 
            break;
        }
        if (p->current->value && strcmp(p->current->value, "}") == 0) break;

        parser_next_token(p);
    }
}

int parser_expect(Parser *p, const char *valor_esperado, const char *mensagem_erro)
{
    if (p->current && p->current->value && strcmp(p->current->value, valor_esperado) == 0)
    {
        parser_next_token(p); // Sucesso! Consome o token esperado
        return 1;
    }
    
    // Se falhar, ativa o Modo de Pânico Geral imediatamente
    int linha = p->current ? p->current->line : 0;
    printf("Erro Sintático na linha %d: %s (Esperado '%s')\n", linha, mensagem_erro, valor_esperado);
    
    parser_sincronizar_geral(p);
    return 0; // Sinaliza falha
}