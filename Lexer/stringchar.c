#include "token.h"

char *process_identifier_string_char(char *line, t_token **tokens, int row)
{
    char quote = line[0];
    int i = 1;
    t_token *new_token;

    while (line[i] && line[i] != quote)
    {
        if (line[i] == '\\' && line[i + 1] != '\0')
            i += 2;
        else
            i++;
    }

    if (line[i] == quote)
    {
        // validação de CHAR
        if (quote == '\'' && i > 2 && line[1] != '\\')
        {
            printf("Erro léxico: char inválido na linha %d\n", row);
            return line + i + 1;
        }

        char *token_value = strndup(line, i + 1);
        char *token_type = (quote == '"') ? "STRING_LITERAL" : "CHAR_LITERAL";

        generate_token(tokens, token_value, token_type, row);
        free(token_value);

        return line + i + 1;
    }
    else
    {
        printf("Erro léxico: aspas não fechadas na linha %d\n", row);
        return line + i;
    }
}
char *process_division_operator_and_coment(char *line, t_token **tokens, int row, int fd)
{
    char *token_value;
    char *token_type = "DIVISION_OP";
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
        /* Comentário de bloco: buscamos apenas dentro da linha atual. If not closed, produce COMMENT token with rest of line. */
        int start = 0; // after '/*'
        i = 2; // skip '/*'
        while (line[i] && !(line[i] == '*' && line[i + 1] == '/')) i++;
        if (line[i] == '*' && line[i + 1] == '/') {
            token_type = "COMMENT";
            token_value = strndup(line, i + 2);
            generate_token(tokens, token_value, token_type, row);
            free(token_value);
            return line + i + 2;
        } else {
            // unclosed block comment on this line
            token_type = "COMMENT";
            token_value = strndup(line, size);
            generate_token(tokens, token_value, token_type, row);
            free(token_value);
            return line + size;
        }
    }

    token_value = strndup(line, 1); // Apenas o operador de divisão
    token_type = "DIVISION_OP";
    generate_token(tokens, token_value, token_type, row);
    free(token_value);
    return line + i;
}