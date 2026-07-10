#include "token.h"

char *process_numeric_literal(char *line, t_token **tokens, int row)
{
    int i = 0;
    int is_float = 0;

    // 1. Consumir todos os dígitos da parte inteira
    while (line[i] && isdigit((unsigned char)line[i])) {
        i++;
    }

    // 2. Verificar se existe um ponto flutuante legítimo (ex: 9.8)
    if (line[i] == '.' && isdigit((unsigned char)line[i + 1])) {
        is_float = 1;
        i++; // Consome o ponto '.'
        
        // Consumir os dígitos da parte fracionária
        while (line[i] && isdigit((unsigned char)line[i])) {
            i++;
        }
    }

    // 3. Isolar a string numérica completa da memória
    char *token_value = strndup(line, i);
    char *token_type;

    if (is_float) {
        token_type = strdup("FLOAT_LITERAL");
    } else {
        token_type = strdup("INT_LITERAL");
    }

    // 4. Gerar o token de forma unificada
    generate_token(tokens, token_value, token_type, row);

    // Libertar memória local temporária
    free(token_value);
    free(token_type);

    // Retorna o ponteiro posicionado exatamente APÓS o último dígito consumido
    return line + i;
}