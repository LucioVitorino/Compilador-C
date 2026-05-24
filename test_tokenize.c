#include "Lexer/token.h"
#include <stdio.h>

int main()
{
    t_token *tokens = NULL;
    int fd = open("testes/test_operators.c", O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }
    tokens = tokenize(fd);
    
    int count = 0;
    for (t_token *tok = tokens; tok; tok = tok->next) {
        printf("%d: [%s] = '%s' (line %d)\n", count++, tok->type, tok->value, tok->line);
        if (count > 200) {
            printf("... (truncated)\n");
            break;
        }
    }
    
    token_clear_list(&tokens);
    close(fd);
    return 0;
}
