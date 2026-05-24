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

void parser_sincronizar(Parser *p)
{
    // Avança e descarta tokens até encontrar um ponto seguro de sincronização
    while (p->current)
    {
        // Se bater no Fim do Ficheiro, interrompe imediatamente
        if (p->current->type && strcmp(p->current->type, "TOK_EOF") == 0)
            break;

        // Se encontrar um ponto e vírgula, consome-o. O pânico termina aqui.
        if (p->current->value && strcmp(p->current->value, ";") == 0)
        {
            parser_next_token(p);
            break;
        }

        // Se encontrar uma chaveta de fecho, NÃO a consome.
        // Deixa que a função 'parse_bloco' a apanhe para fechar o escopo corretamente.
        if (p->current->value && strcmp(p->current->value, "}") == 0)
        {
            break;
        }

        // Caso contrário, descarta o token inválido
        parser_next_token(p);
    }
}