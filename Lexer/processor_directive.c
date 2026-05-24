#include "token.h"

char *process_preprocessor_directive(char *line, t_token **tokens, int row)
{
    int i = 0; 
    // Emit TOK_HASH
    char *tokv = strndup("#", 1);
    generate_token(tokens, tokv, "TOK_HASH", row);
    free(tokv);

    i = 1;
    // skip whitespace
    while (line[i] && isspace((unsigned char)line[i])) i++;

    // read directive name (letters)
    int start = i;
    while (line[i] && isalpha((unsigned char)line[i])) i++;
    if (i > start) {
        char *name = strndup(&line[start], i - start);
        // special-case include
        if (strcmp(name, "include") == 0) {
            generate_token(tokens, strdup(name), "TOK_INCLUDE", row);
            free(name);
            while (line[i] && isspace((unsigned char)line[i])) i++;
            if (line[i] == '<') {
                generate_token(tokens, strdup("<"), "TOK_LT", row);
                i++;
                int fstart = i;
                while (line[i] && line[i] != '>') i++;
                char *fname = strndup(&line[fstart], i - fstart);
                generate_token(tokens, fname, "TOK_NOME_FICHEIRO", row);
                free(fname);
                if (line[i] == '>') {
                    generate_token(tokens, strdup(">"), "TOK_GT", row);
                    i++;
                }
            } else if (line[i] == '"') {
                generate_token(tokens, strdup("\""), "TOK_DQUOTE", row);
                i++;
                int fstart = i;
                while (line[i] && line[i] != '"') i++;
                char *fname = strndup(&line[fstart], i - fstart);
                generate_token(tokens, fname, "TOK_NOME_FICHEIRO", row);
                free(fname);
                if (line[i] == '"') {
                    generate_token(tokens, strdup("\""), "TOK_DQUOTE", row);
                    i++;
                }
            } else {
                int rstart = i;
                while (line[i] && line[i] != '\n') i++;
                char *rest = strndup(&line[rstart], i - rstart);
                generate_token(tokens, rest, "TOK_DIRECTIVE_REST", row);
                free(rest);
            }
        } else {
            generate_token(tokens, name, "TOK_DIRECTIVE", row);
            free(name);
            int rstart = i;
            while (line[i] && line[i] != '\n') i++;
            if (i > rstart) {
                char *rest = strndup(&line[rstart], i - rstart);
                generate_token(tokens, rest, "TOK_DIRECTIVE_REST", row);
                free(rest);
            }
        }
    }

    // Avança de forma segura até encontrar o fim da linha ou o caractere nulo
    while (line[i] && line[i] != '\n') {
        i++;
    }
    
    // Se parou no '\n', avançamos também o '\n' para que o lexer mude completamente de contexto
    if (line[i] == '\n') {
        i++;
    }

    return line + i;
}