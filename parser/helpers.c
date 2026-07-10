#include "parser.h"

static int parse_asteriscos_internal(Parser *p)
{
    int count = 0;
    while (p->current && p->current->value && strcmp(p->current->value, "*") == 0) {
        parser_next_token(p);
        count++;
    }
    return count;
}

static int parse_sufixo_array_opcional_internal(Parser *p)
{
    int count = 0;
    while (p->current && p->current->value && strcmp(p->current->value, "[") == 0) {
        parser_next_token(p);
        if (p->current && !(p->current->value && strcmp(p->current->value, "]") == 0)) {
            parse_expressao(p);
        }
        if (p->current && p->current->value && strcmp(p->current->value, "]") == 0) {
            parser_next_token(p);
        }
        count++;
    }
    return count;
}

static ASTNode *parse_parametro_internal(Parser *p)
{
    ASTNode *tipo = parse_especificador_tipo(p);
    if (!tipo) return NULL;

    int stars = parse_asteriscos_internal(p);

    ASTNode *param = make_node(NODE_DECLARACAO_VARIAVEL, 0);
    if (p->current && p->current->type && strcmp(p->current->type, "IDENTIFIER") == 0) {
        param->valor = strdup(p->current->value);
        parser_next_token(p);
    }
    // possible array suffix
    parse_sufixo_array_opcional_internal(p);
    // attach type as child
    add_filho(param, tipo);
    (void)stars;
    return param;
}

// Função auxiliar interna para processar a lista de parâmetros encadeados por vírgulas
static void parse_lista_parametros_internal(Parser *p, ASTNode *func_node)
{
    do {
        // Lê o tipo do parâmetro (ex: int, float, char)
        ASTNode *tipo_param = parse_especificador_tipo(p);
        if (!tipo_param) {
            syntax_error_recover(p, "Esperado um tipo de dado válido no parâmetro", SYNC_DECLARACAO);
            break;
        }

        // Processa ponteiros opcionais (ex: int *ptr)
        int stars = parse_asteriscos(p);

        // Lê o nome/identificador do parâmetro (ex: a, b)
        if (p->current && p->current->type && strcmp(p->current->type, "IDENTIFIER") == 0)
        {
            const char *p_name = p->current->value ? p->current->value : "";
            int p_line = p->current->line;
            
            ASTNode *param_node = make_folha(NODE_DECLARACAO_VARIAVEL, p_name, p_line);
            param_node->pointer_level = stars;
            
            //Une o tipo do dado ao nó do parâmetro
            add_filho(param_node, tipo_param);
            
            //ANEXA o parâmetro diretamente como filho do nó da função pai!
            add_filho(func_node, param_node);
            
            parser_next_token(p); // Consome o IDENTIFIER
            
            // Suporte opcional a arrays (ex: int arr[])
            int dims = parse_sufixo_array_opcional(p);
            param_node->dimensions = dims;
        }
        else
        {
            syntax_error_recover(p, "Esperado identificador do parâmetro após o tipo", SYNC_DECLARACAO);
            break;
        }

    } while (p->current && p->current->value && strcmp(p->current->value, ",") == 0 && (parser_next_token(p), 1));
}

// Função principal de tratamento de parâmetros na assinatura
void parse_parametros_opcionais(Parser *p, ASTNode *func_node)
{
    if (!p->current || !func_node) return;

    // Se o próximo token for ')', significa uma lista vazia: int main()
    if (p->current->value && strcmp(p->current->value, ")") == 0) {
        return;
    }

    // Mapeia o caso 'TOKEN_VOID' explícito da gramática: int soma(void)
    if (p->current->type && strcmp(p->current->type, "KEYWORD_VOID") == 0) {
        int line = p->current->line;
        parser_next_token(p); // consome 'void'

        // Cria uma folha indicando tipo void e anexa diretamente à função
        ASTNode *void_node = make_folha(NODE_ESPECIFICADOR_TIPO, "void", line);
        add_filho(func_node, void_node);
        return;
    }

    // Se houver parâmetros válidos (ex: int a, int b), processa a lista normal
    parse_lista_parametros_internal(p, func_node); 
}

/* Expose wrappers used from other parser modules */
int parse_asteriscos(Parser *p) { return parse_asteriscos_internal(p); }
int parse_sufixo_array_opcional(Parser *p) { return parse_sufixo_array_opcional_internal(p); }
