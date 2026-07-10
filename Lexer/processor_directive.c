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
        } else if (strcmp(name, "define") == 0) {
            generate_token(tokens, strdup(name), "TOK_DEFINE", row);
            free(name);
            
            // Skip spaces
            while (line[i] && isspace((unsigned char)line[i]) && line[i] != '\n') i++;
            
            // Read macro identifier name
            int id_start = i;
            while (line[i] && (isalnum((unsigned char)line[i]) || line[i] == '_')) i++;
            if (i > id_start) {
                char *id_val = strndup(&line[id_start], i - id_start);
                generate_token(tokens, id_val, "IDENTIFIER", row);
                free(id_val);
            }
            
            // Skip spaces
            while (line[i] && isspace((unsigned char)line[i]) && line[i] != '\n') i++;
            
            // Read optional macro value
            if (line[i] && line[i] != '\n' && line[i] != '\r') {
                // If it's a string literal
                if (line[i] == '"') {
                    int val_start = i;
                    i++;
                    while (line[i] && line[i] != '"') i++;
                    if (line[i] == '"') i++;
                    char *val_str = strndup(&line[val_start], i - val_start);
                    generate_token(tokens, val_str, "STRING_LITERAL", row);
                    free(val_str);
                }
                // If it's a char literal
                else if (line[i] == '\'') {
                    int val_start = i;
                    i++;
                    while (line[i] && line[i] != '\'') i++;
                    if (line[i] == '\'') i++;
                    char *val_str = strndup(&line[val_start], i - val_start);
                    generate_token(tokens, val_str, "CHAR_LITERAL", row);
                    free(val_str);
                }
                // If it's numeric literal
                else if (isdigit((unsigned char)line[i])) {
                    int val_start = i;
                    int is_float = 0;
                    while (line[i] && (isdigit((unsigned char)line[i]) || line[i] == '.')) {
                        if (line[i] == '.') is_float = 1;
                        i++;
                    }
                    char *val_str = strndup(&line[val_start], i - val_start);
                    generate_token(tokens, val_str, is_float ? "FLOAT_LITERAL" : "INT_LITERAL", row);
                    free(val_str);
                }
                // If it's an identifier
                else if (isalpha((unsigned char)line[i]) || line[i] == '_') {
                    int val_start = i;
                    while (line[i] && (isalnum((unsigned char)line[i]) || line[i] == '_')) i++;
                    char *val_str = strndup(&line[val_start], i - val_start);
                    generate_token(tokens, val_str, "IDENTIFIER", row);
                    free(val_str);
                }
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

    // Avança até encontrar o fim da linha ou o caractere nulo
    while (line[i] && line[i] != '\n') {
        i++;
    }
    
    // Se parou no '\n', avançamos também o '\n' para que o lexer mude completamente de contexto
    if (line[i] == '\n') {
        i++;
    }

    return line + i;
}