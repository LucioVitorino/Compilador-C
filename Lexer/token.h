#ifndef TOKEN_H
# define TOKEN_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

#define BUFFER_SIZE 1024

typedef struct s_token{
    char *value;
    char *type;
    int line;
    struct s_token *next;
} t_token;


t_token *token_create(const char *value, char *type, int line);
t_token *token_insert_back_new(t_token **head, const char *value, char *  type, int line);
t_token *tokenize(int fd);
int     ft_strlen(const char *s);
char    *get_next_line(int fd);
char    *ft_strjoin(const char *s1, const char *s2);
char *determine_token_type(char *token_value);
char *process_identifier_keyWord(char *line, t_token **tokens, int row);
char *process_identifier_string_char(char *line, t_token **tokens, int row);
char *process_arithmetic_operator(char *line, t_token **tokens, int row);
char *process_equals_sinal(char *line, t_token **tokens, int row);
char *process_assignment_comparison_operator(char *line, t_token **tokens, int row);
char *process_numeric_literal(char *line, t_token **tokens, int row);
char *process_logical_bitwise_operator(char *line, t_token **tokens, int row);
char *process_delimiter(char *line, t_token **tokens, int row);
char *process_division_operator_and_coment(char *line, t_token **tokens, int row, int fd);
char *process_preprocessor_directive(char *line, t_token **tokens, int row);
void token_insert_back(t_token **head, t_token *new_token);
void token_print_list(t_token *head);
void token_clear_list(t_token **head);
void token_print_list(t_token *head);
void token_clear_list(t_token **head);
void generate_token(t_token **tokens, char *lexema, char *token_type, int row);

#endif