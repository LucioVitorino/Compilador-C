#include "tabela_simbolos.h"
#include "ast/ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Função auxiliar para converter o nome do tipo da AST para o Enum da Tabela
TipoDado mapear_tipo(const char *tipo_str) {
    if (!tipo_str) return TIPO_VOID;
    if (strcmp(tipo_str, "int") == 0) return TIPO_INT;
    if (strcmp(tipo_str, "float") == 0) return TIPO_FLOAT;
    if (strcmp(tipo_str, "char") == 0) return TIPO_CHAR;
    return TIPO_VOID;
}

// Percorre a AST recursivamente para povoar a Tabela de Símbolos
void analisar_semantica(ASTNode *node, SymbolTable *st, EscopoSimbolo escopo_atual) {
    if (!node) return;

    // Caso 1: Encontrou uma declaração de variável
    if (node->type == NODE_DECLARACAO_VARIAVEL) {
        Symbol *sym = (Symbol *)calloc(1, sizeof(Symbol));
        
        sym->lexeme = strdup(node->valor ? node->valor : "");
        sym->token = strdup("IDENTIFIER");
        sym->linha = node->linha;
        sym->categoria = CAT_VARIAVEL;
        
        // Define o escopo e o tipo de variável (Local, Global ou Atributo)
        sym->escopo = escopo_atual;
        if (escopo_atual == ESCOPO_CLASSE) {
            sym->tipo_var = VAR_ATRIBUTO;  // <=== AGORA DETETA COMO ATRIBUTO!
        } else if (escopo_atual == ESCOPO_METODO) {
            sym->tipo_var = VAR_LOCAL;
        } else {
            sym->tipo_var = VAR_GLOBAL;
        }

        // Mapear o Tipo de Dado do primeiro filho
        if (node->n_filhos > 0 && node->filhos[0]->type == NODE_ESPECIFICADOR_TIPO) {
            sym->tipo_dado = mapear_tipo(node->filhos[0]->valor);
            if (sym->tipo_dado == TIPO_INT || sym->tipo_dado == TIPO_FLOAT) {
                sym->tamanho = 4;
            } else if (sym->tipo_dado == TIPO_CHAR) {
                sym->tamanho = 1;
            }
        }

        // Verificar inicialização
        if (node->n_filhos > 1) {
            sym->inicializado = true;
            if (node->filhos[1]->valor) {
                sym->valor = strdup(node->filhos[1]->valor);
            }
        } else {
            sym->inicializado = false;
        }

        if (!insert_symbol(st, sym)) {
            fprintf(stderr, "Erro Semântico (Linha %d): Variável '%s' já declarada neste escopo.\n", sym->linha, sym->lexeme);
            free(sym->token); free(sym->lexeme); if (sym->valor) free(sym->valor); free(sym);
        }
    }
    
    // Caso 2: Encontrou a definição de uma Struct / Typedef (Contexto de Atributos)
    else if (node->type == NODE_DECLARACAO_TYPEDEF) {
        Symbol *sym = (Symbol *)calloc(1, sizeof(Symbol));
        sym->lexeme = strdup(node->valor ? node->valor : "struct_anonima");
        sym->token = strdup("IDENTIFIER");
        sym->linha = node->linha;
        sym->categoria = CAT_CLASSE; // Tratamos a Struct como categoria CLASSE
        sym->escopo = ESCOPO_GLOBAL;

        insert_symbol(st, sym);

        // Criar um escopo isolado para os membros (atributos) da struct
        SymbolTable *escopo_struct = create_table(st);
        
        // Percorrer os filhos da struct definindo o escopo interno como ESCOPO_CLASSE
        for (int i = 0; i < node->n_filhos; i++) {
            analisar_semantica(node->filhos[i], escopo_struct, ESCOPO_CLASSE);
        }
        
        printf("\n--- ESCOPO DA STRUCT/TYPEDEF: %s ---", sym->lexeme);
        print_table(escopo_struct);
        
        free_table(escopo_struct); 
        return;
    }

    // Caso 3: Encontrou uma definição de Função (Método)
   // Caso 3: Encontrou uma definição de Função (Método)
    else if (node->type == NODE_DECLARACAO_FUNCAO) {
        Symbol *sym = (Symbol *)calloc(1, sizeof(Symbol));
        sym->lexeme = strdup(node->valor ? node->valor : "");
        sym->token = strdup("IDENTIFIER");
        sym->linha = node->linha;
        sym->categoria = CAT_METODO;
        sym->escopo = escopo_atual;

        if (node->n_filhos > 0 && node->filhos[0]->type == NODE_ESPECIFICADOR_TIPO) {
            sym->retorno = mapear_tipo(node->filhos[0]->valor);
        }

        if (!insert_symbol(st, sym)) {
            fprintf(stderr, "Erro Semântico (Linha %d): Função '%s' já definida.\n", sym->linha, sym->lexeme);
            free(sym->token); free(sym->lexeme); free(sym);
        }

        SymbolTable *escopo_local = create_table(st);
        
        // Percorre os filhos da função
        for (int i = 0; i < node->n_filhos; i++) {
            if (node->filhos[i]->type == NODE_DECLARACAO_VARIAVEL) {
                // Se o filho for uma variável direta da função, significa que é um PARÂMETRO da assinatura!
                analisar_semantica(node->filhos[i], escopo_local, ESCOPO_METODO);
                
                // Procuramos o símbolo acabado de inserir no escopo local para mudar a categoria
                Symbol *param_sym = lookup_symbol(escopo_local, node->filhos[i]->valor);
                if (param_sym) {
                    param_sym->categoria = CAT_PARAMETRO; // <- Ajusta para o teu enum de categoria (ex: CAT_PARAMETRO ou PARAMETRO)
                }
            } else {
                // Se for o NODE_BLOCO ou o especificador de tipo, segue o fluxo normal
                analisar_semantica(node->filhos[i], escopo_local, ESCOPO_METODO);
            }
        }
        
        printf("\n--- ESCOPO LOCAL DA FUNÇÃO: %s ---", sym->lexeme);
        print_table(escopo_local);
        
        free_table(escopo_local); 
        return;
    }

    // Propagação recursiva padrão para os restantes nós
    for (int i = 0; i < node->n_filhos; i++) {
        analisar_semantica(node->filhos[i], st, escopo_atual);
    }
}