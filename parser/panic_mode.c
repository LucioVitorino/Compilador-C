#include "parser.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Adicionamos tipos de dados e diretivas como âncoras de emergência globais
const char *const SYNC_DECLARACAO[] = {";", "}", "KEYWORD_INT", "KEYWORD_FLOAT", "KEYWORD_CHAR", "KEYWORD_VOID", "TOK_HASH", "TOK_EOF", NULL};
const char *const SYNC_BLOCO[] = {"}", "TOK_EOF", NULL};
const char *const SYNC_INSTRUCAO[] = {";", "}", "KEYWORD_IF", "KEYWORD_WHILE", "KEYWORD_FOR", "KEYWORD_RETURN", "TOK_EOF", NULL};
const char *const SYNC_EXPRESSAO[] = {";", ")", "]", "}", "TOK_EOF", NULL};
const char *const SYNC_HEADER[] = {")", "{", ";", "}", "TOK_EOF", NULL};


static int g_n_errors = 0;

int parser_error_count(void) {
    return g_n_errors;
}

void parser_reset_error_count(void) {
    g_n_errors = 0;
}

void recover(Parser *p, const char *const sync_set[]) {
    int i;
    if (!p || !sync_set) return;

    while (p->current && (!(p->current->type && strcmp(p->current->type, "TOK_EOF") == 0))) {
        for (i = 0; sync_set[i] != NULL; i++) {
            if ((p->current->value && strcmp(p->current->value, sync_set[i]) == 0) ||
                (p->current->type && strcmp(p->current->type, sync_set[i]) == 0)) {
                return; // Sincronizado! Mantém o token âncora como o current.
            }
        }
        parser_next_token(p); // Descarta o token inválido
    }
}

void syntax_error_recover(Parser *p, const char *msg, const char *const sync_set[]) {
    const char *type = "<null>";
    const char *value = "<null>";
    int line = 0;

    if (p && p->current) {
        if (p->current->type) type = p->current->type;
        if (p->current->value) value = p->current->value;
        line = p->current->line;
    }
    
    fprintf(stderr, "\nERRO : linha %d: %s\n  encontrado: tipo=\"%s\" val=\"%s\"\n", line, msg, type, value);
    g_n_errors++;
    recover(p, sync_set);
}
int parser_expect(Parser *p, const char *valor_esperado, const char *mensagem_erro, const char *const sync_set[])
{
    // Caso de sucesso: o token atual é o que esperávamos
    if (p->current && p->current->value && strcmp(p->current->value, valor_esperado) == 0) {
        parser_next_token(p); // Consome o delimitador esperado
        return 1;
    }

    // Caso de falha: O token não foi encontrado! Ativar Modo Pânico imediatamente.
    char msg_completa[256];
    snprintf(msg_completa, sizeof(msg_completa), "%s (Esperado: '%s')", mensagem_erro, valor_esperado);
    
    // Dispara o erro visual no terminal e incrementa g_n_errors
    syntax_error_recover(p, msg_completa, sync_set);
    
    return 0; // Sinaliza que houve uma falha na estrutura
}