#include "token.h"

char *process_identifier_string_char(char *line, t_token **tokens, int row)
{
    int i = 0;
    char quote_type = line[0];
    
    i++;

    // Avança tudo até encontrar a aspa de fecho ou o fim da string/linha
    while (line[i] && line[i] != quote_type) {
        if (line[i] == '\\' && line[i + 1]) {
            i += 2;
        } else {
            i++;
        }
    }

    // Se saiu do loop sem encontrar o fechamento correspondente
    if (line[i] != quote_type) {
        printf("Erro léxico: aspas não fechadas na linha %d\n", row);
        return line + i; // Retorna o ponto atual para evitar loop infinito
    }

    i++; // Avança a aspa/plica de fecho

    // Isola e extrai o literal completo da memória
    char *token_value = strndup(line, i);
    char *token_type;

    if (quote_type == '"') {
        token_type = strdup("STRING_LITERAL");
    } else {
        token_type = strdup("CHAR_LITERAL");
    }

    t_token *new_token = token_create(token_value, token_type, row);
    token_insert_back(tokens, new_token);

    free(token_value);

    // Retorna o ponteiro posicionado exatamente após o fecho da string
    return line + i;
}

char *process_division_operator_and_coment(char *line, t_token **tokens, int row, int fd)
{
    char *token_value;
    char *token_type;
    int i = 1;
    int size = ft_strlen(line);

    if (line[i] == '/')
    {   // Comentário de linha
        token_type = "COMMENT";
        token_value = strndup(line, size);
        generate_token(tokens, token_value, token_type, row);
        free(token_value);
        return line + size;
    }
    else if (line[i] == '*')
    {
        /* Comentário de bloco */
        i = 2; // pula '/*'
        while (line[i] && !(line[i] == '*' && line[i + 1] == '/')) i++;
        if (line[i] == '*' && line[i + 1] == '/') {
            token_type = "COMMENT";
            token_value = strndup(line, i + 2);
            generate_token(tokens, token_value, token_type, row);
            free(token_value);
            return line + i + 2;
        } else {
            token_type = "COMMENT";
            token_value = strndup(line, size);
            generate_token(tokens, token_value, token_type, row);
            free(token_value);
            return line + size;
        }
    }

    if (line[i] == '=')
    {
        token_value = strndup(line, 2); // Captura "/="
        token_type = "ARITHMETIC_ASSIGNMENT_OP"; // Unificado com += e -=
        generate_token(tokens, token_value, token_type, row);
        free(token_value);
        return line + 2; // Avança dois caracteres
    }

    // Caso contrário, é apenas divisão pura '/'
    token_value = strndup(line, 1);
    token_type = "DIVISION_OP";
    generate_token(tokens, token_value, token_type, row);
    free(token_value);
    return line + 1;
}