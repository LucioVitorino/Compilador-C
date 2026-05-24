#include "token.h"

t_token *tokenize(int fd)
{
    t_token *tokens = NULL;
    char *line;
    char *next;
    int i;
    int row = 1;
    while ((line = get_next_line(fd)))
    {
        i = 0;
        while (line[i])
        {

            if (line[i] == ' ' || line[i] == '\t' || line[i] == '\n' || line[i] == '\r')
            {
                i++; // Consome os espaços em branco e avança
                continue;
            }

            int old_i = i;

            if (isalpha(line[i]) || line[i] == '_')
            {
                // lógica para identificar identificadores
                next = process_identifier_keyWord(&line[i], &tokens, row);
                i = (int)(next - line);
            }
            else if (line[i] == '"' || line[i] == '\'')
            {
                // Lógica para reconhecer literais de string e char
                next = process_identifier_string_char(&line[i], &tokens, row);
                i = (int)(next - line);
            }
            else if (line[i] == '+' || line[i] == '-' || line[i] == '*' ||
                     line[i] == '%')
            {
                // Reconhecer operadores aritméticos
                next = process_arithmetic_operator(&line[i], &tokens, row);
                i = (int)(next - line);
            }
            else if (line[i] == '=')
            {
                // Lógica para reconhecer atribuição ou comparação
                next = process_equals_sinal(&line[i], &tokens, row);
                i = (int)(next - line);
            }
            else if (line[i] == '>' || line[i] == '<')
            {
                // lógica para reconhecer >, >=, <=, <, >>=, <<=
                next = process_assignment_comparison_operator(&line[i], &tokens, row);
                i = (int)(next - line);
            }
            else if (isdigit(line[i]))
            {
                // Lógica para reconhecer literais numéricos
                next = process_numeric_literal(&line[i], &tokens, row);
                i = (int)(next - line);
            }
            else if (line[i] == '&' || line[i] == '|' || line[i] == '!' || line[i] == '^' || line[i] == '~')
            {
                // Lógica para reconhecer operadores lógicos e bitwise
                next = process_logical_bitwise_operator(&line[i], &tokens, row);
                i = (int)(next - line);
            }
            else if (line[i] == '(' || line[i] == ')' || line[i] == '{' ||
                     line[i] == '}' || line[i] == '[' || line[i] == ']' ||
                     line[i] == ';' || line[i] == ',' || line[i] == '.')
            {
                // Lógica para reconhecer delimitadores
                next = process_delimiter(&line[i], &tokens, row);
                i = (int)(next - line);
            }
            else if (line[i] == '/')
            {
                next = process_division_operator_and_coment(&line[i], &tokens, row, fd);
                i = (int)(next - line);
            }
            else if (line[i] == '#')
            {
                // Lógica para reconhecer diretivas de pré-processamento
                next = process_preprocessor_directive(&line[i], &tokens, row);
                i = (int)(next - line);
            }
            if (i <= old_i)
            {
                i = old_i + 1;
            }
        }
        row++;
        free(line);
    }

    // Append EOF token
    generate_token(&tokens, "EOF", "TOK_EOF", row);
    return tokens;
}