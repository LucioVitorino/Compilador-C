#include "../Lexer/token.h"

char *get_next_line(int fd)
{
    static char buffer[BUFFER_SIZE];
    static int index = 0, bytes = 0;
    char *line;
    int i = 0;

    if (fd < 0 || !(line = malloc(BUFFER_SIZE + 1)))
        return (NULL);
    while (1)
    {
        if (index >= bytes)
        {
            bytes = read(fd, buffer, BUFFER_SIZE);
            index = 0;
            if (bytes <= 0)
                break ;
        }
        line[i++] = buffer[index];
        if(buffer[index++] == '\n')
            break ;
    }
    if (i == 0)
        return (free(line), NULL);
    line[i] = '\0';
    return (line);
}