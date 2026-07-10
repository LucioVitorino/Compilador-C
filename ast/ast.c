#include "ast.h"

ASTNode *make_node(NodeType type, int linha)
{
    ASTNode *n = malloc(sizeof(ASTNode));
    if (!n) return NULL;
    n->type = type;
    n->valor = NULL;
    n->op = NULL;
    n->linha = linha;
    n->filhos = NULL;
    n->n_filhos = 0;
    n->cap_filhos = 0;
    return n;
}

ASTNode *make_folha(NodeType type, const char *valor, int linha)
{
    ASTNode *n = make_node(type, linha);
    if (!n) return NULL;
    if (valor) n->valor = strdup(valor);
    return n;
}

void add_filho(ASTNode *pai, ASTNode *filho)
{
    if (!pai || !filho) return;
    if (pai->n_filhos == pai->cap_filhos) {
        pai->cap_filhos = pai->cap_filhos ? pai->cap_filhos * 2 : 4;
        pai->filhos = realloc(pai->filhos, pai->cap_filhos * sizeof(ASTNode*));
    }
    pai->filhos[pai->n_filhos++] = filho;
}

void print_ast(ASTNode *n, int depth)
{
    if (!n) return;
    /* Map enum to readable name */
    const char *name;
    switch (n->type) {
        case NODE_PROGRAMA: name = "NODE_PROGRAMA"; break;
        case NODE_LISTA_DECL_GLOBAIS: name = "NODE_LISTA_DECL_GLOBAIS"; break;
        case NODE_DIRETIVA_INCLUDE: name = "NODE_DIRETIVA_INCLUDE"; break;
        case NODE_DIRETIVA_DEFINE: name = "NODE_DIRETIVA_DEFINE"; break;
        case NODE_DECLARACAO_TYPEDEF: name = "NODE_DECLARACAO_TYPEDEF"; break;
        case NODE_DECLARACAO_VARIAVEL: name = "NODE_DECLARACAO_VARIAVEL"; break;
        case NODE_DECLARACAO_FUNCAO: name = "NODE_DECLARACAO_FUNCAO"; break;
        case NODE_BLOCO: name = "NODE_BLOCO"; break;
        case NODE_IF: name = "NODE_IF"; break;
        case NODE_WHILE: name = "NODE_WHILE"; break;
        case NODE_DO_WHILE: name = "NODE_DO_WHILE"; break;
        case NODE_FOR: name = "NODE_FOR"; break;
        case NODE_SWITCH: name = "NODE_SWITCH"; break;
        case NODE_CASE: name = "NODE_CASE"; break;
        case NODE_RETURN: name = "NODE_RETURN"; break;
        case NODE_BREAK: name = "NODE_BREAK"; break;
        case NODE_CONTINUE: name = "NODE_CONTINUE"; break;
        case NODE_INSTR_EXPR: name = "NODE_INSTR_EXPR"; break;
        case NODE_ATRIBUICAO: name = "NODE_ATRIBUICAO"; break;
        case NODE_OP_BINARIO: name = "NODE_OP_BINARIO"; break;
        case NODE_OP_UNARIO: name = "NODE_OP_UNARIO"; break;
        case NODE_POS_FIXO: name = "NODE_POS_FIXO"; break;
        case NODE_CHAMADA_FUNC: name = "NODE_CHAMADA_FUNC"; break;
        case NODE_ACESSO_ARRAY: name = "NODE_ACESSO_ARRAY"; break;
        case NODE_ACESSO_MEMBRO: name = "NODE_ACESSO_MEMBRO"; break;
        case NODE_IDENTIFICADOR: name = "NODE_IDENTIFICADOR"; break;
        case NODE_ESPECIFICADOR_TIPO: name = "NODE_ESPECIFICADOR_TIPO"; break;
        case NODE_NOME_FICHEIRO: name = "NODE_NOME_FICHEIRO"; break;
        case NODE_LIT_INTEIRO: name = "NODE_LIT_INTEIRO"; break;
        case NODE_LIT_REAL: name = "NODE_LIT_REAL"; break;
        case NODE_LIT_CHAR: name = "NODE_LIT_CHAR"; break;
        case NODE_LIT_STRING: name = "NODE_LIT_STRING"; break;
        default: name = "NODE_UNKNOWN"; break;
    }
    for (int i = 0; i < depth; ++i) printf("  ");
    /* Print node header */
    printf("[%s] (L%d)", name, n->linha);

    /* If node has a direct value or op, append them on the same line */
    if (n->valor) printf(" val=\"%s\"", n->valor);
    if (n->op && n->type != NODE_NOME_FICHEIRO) printf(" op=\"%s\"", n->op);

    /* Special formatting for include filename kinds */
    if (n->type == NODE_NOME_FICHEIRO && n->valor) {
        if (n->op && strcmp(n->op, "system") == 0) printf(" %s", "sistema <>");
        else printf(" %s", "local \"\"");
    }

    printf("\n");

    for (int i = 0; i < n->n_filhos; ++i) print_ast(n->filhos[i], depth+1);
}

void free_ast(ASTNode *n)
{
    if (!n) return;
    for (int i = 0; i < n->n_filhos; ++i) free_ast(n->filhos[i]);
    free(n->filhos);
    free(n->valor);
    free(n->op);
    free(n);
}
