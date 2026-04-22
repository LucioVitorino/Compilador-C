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
            printf("%c -- %d", quote, i);
            printf("Erro léxico: char inválido na linha %d\n", row);
            return line + i + 1;
        }

        char *token_value = strndup(line, i + 1);
        char *token_type = (quote == '"') ? "STRING" : "CHAR";

        new_token = token_create(token_value, token_type, row);
        token_insert_back(tokens, new_token);

        // só libera se token_create copiar internamente
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
    {        // Comentário de linha
        token_type = "COMMENT";
        token_value = strndup(line, size);
        generate_token(tokens, token_value, token_type, row);
        free(token_value);
        return line + size;
    }
    else if (line[i] == '*')
    {
    /* Comentário de bloco; acumulamos linhas enquanto não fecharmos '* /'.
         */
        token_value = strdup("");
        i++; // Pula o '*' para começar a verificar o conteúdo do comentário
        while (line[i] && !(line[i] == '*' && line[i + 1] == '/'))
        {
            i++;
            if (line[i] == '\n')
            {
                row++;
                char *part = strndup(line, i); 
                char *joined = ft_strjoin(token_value, part);
                free(token_value);
                free(part);
                token_value = joined;
                char *next = get_next_line(fd);
                if (!next)
                    break;
                line = next;
                i = 0;
            }
        }
        token_type = "COMMENT";
    /* Acrescentar a parte final (inclui '* /' quando presente) */
        char *part = strndup(line, i + 2);
        char *joined = ft_strjoin(token_value, part);
        free(token_value);
        free(part);
        token_value = joined;
        generate_token(tokens, token_value, token_type, row);
        free(token_value);
        return line + i + 2;
    }

    token_value = strndup(line, 1); // Apenas o operador de divisão
    free(token_value);
    return line + i;
}