#include "token.h"

char *process_numeric_literal(char *line, t_token **tokens, int row)
{
    int i = 0;
    char *token_value;
    char *token_type;

    while (line[i] && isdigit((unsigned char)line[i]))
        i++;
    if (line[i] == '.')
    {
        i++;
        while (line[i] && isdigit((unsigned char)line[i]))
            i++;
    }
    token_value = strndup(line, i);
    token_type = strchr(token_value, '.') ? "FLOAT_LITERAL" : "INT_LITERAL";
    generate_token(tokens, token_value, token_type, row);
    free(token_value);

    return line + i;
}