#include "Lexer/token.h"

int main(int argc, char **argv)
{
    int fd;
    t_token *tokens;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <source-file.c>\n", argv[0]);
        return 1;
    }

    fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("Error opening file");
        return 1;
    }

    tokens = tokenize(fd);
    token_print_list(tokens);
    token_clear_list(&tokens);
    close(fd);

}