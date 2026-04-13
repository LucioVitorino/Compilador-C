#include "token.h"

char *process_preprocessor_directive(char *line, t_token **tokens, int row)
{
    int i = 1;
    char *token_value;
    char *token_type = "PREPROCESSOR_DIRECTIVE";

    while (line[i] && line[i] != '\n')
        i++;

    token_value = strndup(line, i);
    generate_token(tokens, token_value, token_type, row);
    free(token_value);

    return line + i;
}