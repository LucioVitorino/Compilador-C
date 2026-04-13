#include "token.h"

char *process_identifier_keyWord(char *line, t_token **tokens, int row)
{
    int i = 0;
    t_token *new_token;

        while (line[i] && (isalpha((unsigned char)line[i]) || line[i] == '_'))
            i++;

    char *token_value = strndup(line, i);
    char *token_type = determine_token_type(token_value);
    new_token = token_create(token_value, token_type, row);
    token_insert_back(tokens, new_token);
    free(token_value);
    
    return line + i;
}

char *determine_token_type(char *token_value)
{
    char word[256];
    FILE *keywords_file = fopen("keyWords.txt", "r");
    if (!keywords_file) {
        perror("Failed to open keywords file");
        return "ERROR";
    }
    while (fgets(word, sizeof(word), keywords_file)) {
        size_t len = strlen(word);
        if (len > 0 && word[len - 1] == '\n')
            word[len - 1] = '\0';
        if (strcmp(token_value, word) == 0) {
            fclose(keywords_file);
            return "KEYWORD";
        }
    }
    fclose(keywords_file);
    return "IDENTIFIER";
}