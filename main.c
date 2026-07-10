#include "Lexer/token.h"
#include "parser/parser.h"
#include "ast/ast.h"
#include "semantico/tabela_simbolos.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    int         fd;
    t_token     *tokens;
    Parser      p;
    SymbolTable *symbol_table;

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
    //token_print_list(tokens);

    // 1. Integrar e Executar Parser Sintático
    parser_init(&p, tokens);
    ASTNode *ast = parse_programa(&p);
    //print_ast(ast, 0);

    // Se houver erros sintáticos, paramos imediatamente antes da semântica
    if (parser_error_count() > 0) {
        fprintf(stderr, "Total de erros: %d\n", parser_error_count());
        free_ast(ast);
        token_clear_list(&tokens);
        close(fd);
        return 1;
    }

    //printf("\n\n=== ANÁLISE SEMÂNTICA ===\n\n");

    // 2. Inicializar a Tabela de Símbolos Global (Pai = NULL)
    symbol_table = create_table(NULL);
    if (!symbol_table) {
        fprintf(stderr, "Erro crítico: Não foi possível alocar a Tabela de Símbolos.\n");
        free_ast(ast);
        token_clear_list(&tokens);
        close(fd);
        return 1;
    }

    // 3. Executar a Verificação Semântica preenchendo a tabela
    // (Esta é a função que vai navegar pelos nós da AST)
    //printf("A varrer a árvore e a construir o mapa de símbolos...\n");
    analisar_semantica(ast, symbol_table, ESCOPO_GLOBAL);

    int sem_errors = obter_numero_erros_semanticos();
    if (sem_errors > 0) {
        fprintf(stderr, "Total de erros: %d\n", sem_errors);
        free_table(symbol_table);
        free_ast(ast);
        token_clear_list(&tokens);
        close(fd);
        return 1;
    }

    // === LIMPEZA DE MEMÓRIA ===
    free_table(symbol_table);
    free_ast(ast);
    token_clear_list(&tokens);
    close(fd);
    printf("sucesso\n");
    return 0;
}