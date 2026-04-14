#include "token.h"

char *process_arithmetic_operator(char *line, t_token **tokens, int row)
{
    int i = 1;
    char operator_char = line[0];
     char *token_value;
     char *token_type;

    // Detectar operadores duplos (++ e --)
    if ((operator_char == '+' || operator_char == '-') && (line[1] == operator_char || line[1] == '='))
        i = 2;

    token_value = strndup(line, i);
    if (strcmp(token_value, "++") == 0 )
        token_type = "INCREMENT_OP";
    else if (strcmp(token_value, "--") == 0)
        token_type = "DECREMENT_OP";
    else if (token_value[1] == '=')
        token_type = "ARITHMETIC_ASSIGNMENT_OP";
    else
        token_type = "ARITHMETIC_OP";

    generate_token(tokens, token_value, token_type, row);
    free(token_value);

    return line + i;
}


char *process_equals_sinal(char *line, t_token **tokens, int row)
{
    int i = 1;
    char *lexema;
    char *token_type;

    if (line[i] == '=')
    {
        i++;
        token_type = "COMPARISON_OPERATOR";
    }
    else 
        token_type = "ASSIGMENT_OPERATOR";
    lexema = strndup(line, i);
    generate_token(tokens, lexema, token_type, row);
    free(lexema);
    return (line + i);
}

char *process_assignment_comparison_operator(char *line, t_token **tokens, int row)
{
    int i = 1;
    char *token_type;
    char *lexema;

    if (!line[i])
        return (line + i);

    if (line[i] == '=')
    {
        i++;
        token_type = "COMPARISON_OPERATOR";
    }
    else if ((line[i] == '>' && line[i - 1] == '>') || (line[i] == '<' && line[i - 1] == '<'))
    {
        i++;
        if (line[i] == '=')
            i++;
        token_type = "BIT_OPERATOR";
    }
    else
        token_type = "COMPARISON_OPERATOR";

    lexema = strndup(line, i);
    generate_token(tokens, lexema, token_type, row);
    free(lexema);
    return (line + i);
}

char *process_logical_bitwise_operator(char *line, t_token **tokens, int row)
{
    int i = 1;
    char *token_type;
    char *lexema;

    if (line[i] == line[0]) // && ou ||
    {
        i++;
        token_type = "LOGICAL_OPERATOR";
    }
    else if (line[0] == '!')
        token_type = "LOGICAL_OPERATOR";
    else
        token_type = "BIT_OPERATOR";

    lexema = strndup(line, i);
    generate_token(tokens, lexema, token_type, row);
    free(lexema);
    return (line + i);
}

char *process_delimiter(char *line, t_token **tokens, int row)
{
    char *token_value;
    char *token_type;

    token_value = strndup(line, 1);
    token_type = "DELIMITER";
    generate_token(tokens, token_value, token_type, row);
    free(token_value);

    return line + 1;
}