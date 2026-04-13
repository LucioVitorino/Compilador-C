#include "../Lexer/token.h"

t_token *token_create(const char *value, char *type, int line)
{
	t_token *new_token;
	size_t value_len;

	new_token = (t_token *)malloc(sizeof(t_token));
	if (!new_token)
		return NULL;
	if (value)
	{
		value_len = strlen(value);
		new_token->value = (char *)malloc(value_len + 1);
		if (!new_token->value)
		{
			free(new_token);
			return NULL;
		}
		memcpy(new_token->value, value, value_len + 1);
	}
	else
		new_token->value = NULL;
	new_token->type = type;
	new_token->line = line;
	new_token->next = NULL;
	return new_token;
}

void token_insert_back(t_token **head, t_token *new_token)
{
	t_token *current;

	if (!head || !new_token)
		return;
	if (!*head)
	{
		*head = new_token;
		return;
	}
	current = *head;
	while (current->next)
		current = current->next;
	current->next = new_token;
}

t_token *token_insert_back_new(t_token **head, const char *value, char *type, int line)
{
	t_token *new_token;

	new_token = token_create(value, type, line);
	if (!new_token)
		return NULL;
	token_insert_back(head, new_token);
	return new_token;
}

void token_print_list(t_token *head)
{
	t_token *current;

	current = head;
	while (current)
	{
		printf("value= %s, type= %s, line= %d\n",
			current->value ? current->value : "(null)",
			current->type,
			current->line);
		current = current->next;
	}
}

void token_clear_list(t_token **head)
{
	t_token *current;
	t_token *next;

	if (!head)
		return;
	current = *head;
	while (current)
	{
		next = current->next;
		free(current->value);
		free(current);
		current = next;
	}
	*head = NULL;
}

void generate_token(t_token **tokens, char *lexema, char *token_type, int row)
{
	t_token *new_token;

	new_token = token_create(lexema, token_type, row);
    token_insert_back(tokens, new_token);
}