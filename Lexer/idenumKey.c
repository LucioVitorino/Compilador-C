#include "token.h"

char *process_identifier_keyWord(char *line, t_token **tokens, int row)
{
    int i = 0;
    t_token *new_token;

    while (line[i] && (isalnum((unsigned char)line[i]) || line[i] == '_'))
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
    // Tabela estática de palavras-chave nativas do C mapeadas na tua gramática
    static const char *keywords[] = {
        "int", "float", "char", "void", "return", 
        "if", "else", "while", "for", "do", 
        "struct", "typedef"
    };
    
    static const char *token_types[] = {
        "KEYWORD_INT", "KEYWORD_FLOAT", "KEYWORD_CHAR", "KEYWORD_VOID", "KEYWORD_RETURN", 
        "KEYWORD_IF", "KEYWORD_ELSE", "KEYWORD_WHILE", "KEYWORD_FOR", "KEYWORD_DO", 
        "KEYWORD_STRUCT", "KEYWORD_TYPEDEF"
    };
    
    int num_keywords = sizeof(keywords) / sizeof(keywords[0]);
    
    for (int i = 0; i < num_keywords; i++) {
        if (strcmp(token_value, keywords[i]) == 0) {
            return strdup(token_types[i]);
        }
    }
    
    // Se não for correspondente a nenhuma palavra-chave, é um identificador válido
    return strdup("IDENTIFIER");
}