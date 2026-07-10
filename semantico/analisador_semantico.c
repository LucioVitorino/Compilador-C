#include "tabela_simbolos.h"
#include "ast/ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* =========================================================================
   VARIÁVEIS GLOBAIS DE CONTROLE SEMÂNTICO
   ========================================================================= */
static int semantic_errors = 0;
static TipoDado esperado_retorno = TIPO_VOID;

/* =========================================================================
   PROTÓTIPOS DAS FUNÇÕES AUXILIARES / INTERNAS (STATIC)
   ========================================================================= */
static TipoDado mapear_tipo(const char *tipo_str, SymbolTable *st);
static void extrair_parametros(ASTNode *func_node, Symbol *sym);

// Resolução de tipos para sub-expressões
static TipoDado resolver_identificador(ASTNode *node, SymbolTable *st);
static TipoDado resolver_op_binario(ASTNode *node, SymbolTable *st);
static TipoDado resolver_op_unario(ASTNode *node, SymbolTable *st);
static TipoDado resolver_pos_fixo(ASTNode *node, SymbolTable *st);
static TipoDado resolver_acesso_array(ASTNode *node, SymbolTable *st);
static TipoDado resolver_acesso_membro(ASTNode *node, SymbolTable *st);
static TipoDado resolver_chamada_func(ASTNode *node, SymbolTable *st);
static TipoDado resolver_atribuicao(ASTNode *node, SymbolTable *st);

// Validação de declarações e estruturas de fluxo
static void analisar_decl_variavel(ASTNode *node, SymbolTable *st, EscopoSimbolo escopo_atual);
static void analisar_decl_typedef(ASTNode *node, SymbolTable *st);
static void analisar_decl_funcao(ASTNode *node, SymbolTable *st, EscopoSimbolo escopo_atual);
static void analisar_if(ASTNode *node, SymbolTable *st, EscopoSimbolo escopo_atual);
static void analisar_while_do_while(ASTNode *node, SymbolTable *st, EscopoSimbolo escopo_atual);
static void analisar_for(ASTNode *node, SymbolTable *st, EscopoSimbolo escopo_atual);
static void analisar_return(ASTNode *node, SymbolTable *st);

/* =========================================================================
   FUNÇÕES PÚBLICAS (API DO ANALISADOR)
   ========================================================================= */

/**
 * Retorna o número total de erros semânticos detetados durante a análise.
 */
int obter_numero_erros_semanticos(void) {
    return semantic_errors;
}

/**
 * Ponto de entrada recursivo para efetuar a verificação semântica em nós da AST.
 * 
 * @param node O nó atual da AST.
 * @param st A tabela de símbolos para o escopo atual.
 * @param escopo_atual O tipo de escopo atual (global, classe, local/método).
 */
void analisar_semantica(ASTNode *node, SymbolTable *st, EscopoSimbolo escopo_atual) {
    if (!node) return;

    switch (node->type) {
        case NODE_DECLARACAO_VARIAVEL:
            analisar_decl_variavel(node, st, escopo_atual);
            break;
            
        case NODE_DECLARACAO_TYPEDEF:
            analisar_decl_typedef(node, st);
            break;
            
        case NODE_DECLARACAO_FUNCAO:
            analisar_decl_funcao(node, st, escopo_atual);
            break;
            
        case NODE_INSTR_EXPR:
            if (node->n_filhos > 0) {
                obter_tipo_expressao(node->filhos[0], st);
            }
            break;
            
        case NODE_IF:
            analisar_if(node, st, escopo_atual);
            break;
            
        case NODE_WHILE:
        case NODE_DO_WHILE:
            analisar_while_do_while(node, st, escopo_atual);
            break;
            
        case NODE_FOR:
            analisar_for(node, st, escopo_atual);
            break;
            
        case NODE_RETURN:
            analisar_return(node, st);
            break;
            
        default:
            // Propagação recursiva padrão para nós genéricos
            for (int i = 0; i < node->n_filhos; i++) {
                analisar_semantica(node->filhos[i], st, escopo_atual);
            }
            break;
    }
}

/**
 * Resolve recursivamente o tipo de uma expressão, verificando compatibilidades lógicas.
 * 
 * @param node O nó da expressão.
 * @param st A tabela de símbolos ativa.
 * @return O tipo de dado resultante (TipoDado).
 */
TipoDado obter_tipo_expressao(ASTNode *node, SymbolTable *st) {
    if (!node) return TIPO_VOID;

    switch (node->type) {
        case NODE_LIT_INTEIRO: return TIPO_INT;
        case NODE_LIT_REAL:    return TIPO_FLOAT;
        case NODE_LIT_CHAR:    return TIPO_CHAR;
        case NODE_LIT_STRING:  return TIPO_STRING;
        case NODE_VAZIO:       return TIPO_VOID;
        
        case NODE_IDENTIFICADOR: return resolver_identificador(node, st);
        case NODE_OP_BINARIO:    return resolver_op_binario(node, st);
        case NODE_OP_UNARIO:     return resolver_op_unario(node, st);
        case NODE_POS_FIXO:      return resolver_pos_fixo(node, st);
        case NODE_ACESSO_ARRAY:  return resolver_acesso_array(node, st);
        case NODE_ACESSO_MEMBRO: return resolver_acesso_membro(node, st);
        case NODE_CHAMADA_FUNC:  return resolver_chamada_func(node, st);
        case NODE_ATRIBUICAO:    return resolver_atribuicao(node, st);
        
        default:                 return TIPO_VOID;
    }
}

/**
 * Extrai e popula a lista de parâmetros de uma função na AST.
 */
static void extrair_parametros(ASTNode *func_node, Symbol *sym) {
    ParamNode *last_param = NULL;
    for (int i = 1; i < func_node->n_filhos; i++) {
        ASTNode *child = func_node->filhos[i];
        if (child->type == NODE_DECLARACAO_VARIAVEL) {
            ParamNode *p = (ParamNode *)calloc(1, sizeof(ParamNode));
            if (!p) return;
            p->nome = strdup(child->valor ? child->valor : "");
            if (child->n_filhos > 0 && child->filhos[0]->type == NODE_ESPECIFICADOR_TIPO) {
                p->tipo = mapear_tipo(child->filhos[0]->valor, NULL);
            } else {
                p->tipo = TIPO_INT;
            }
            p->next = NULL;
            if (!sym->parametros) {
                sym->parametros = p;
            } else {
                last_param->next = p;
            }
            last_param = p;
        }
    }
}

/* =========================================================================
   IMPLEMENTAÇÃO DAS FUNÇÕES AUXILIARES DE RESOLUÇÃO DE EXPRESSÕES (STATIC)
   ========================================================================= */

/**
 * Resolve e valida o tipo de um identificador na tabela de símbolos.
 */
static TipoDado resolver_identificador(ASTNode *node, SymbolTable *st) {
    Symbol *sym = lookup_symbol(st, node->valor);
    if (!sym) {
        fprintf(stderr, "Erro Semântico (Linha %d): Variável '%s' não declarada.\n", node->linha, node->valor);
        semantic_errors++;
        return TIPO_VOID;
    }
    if (sym->categoria == CAT_METODO) {
        return sym->retorno;
    }
    return sym->tipo_dado;
}

/**
 * Resolve e valida o tipo de operações binárias (+, -, *, /, ==, <, etc.).
 */
static TipoDado resolver_op_binario(ASTNode *node, SymbolTable *st) {
    if (node->n_filhos < 2) return TIPO_VOID;
    TipoDado t1 = obter_tipo_expressao(node->filhos[0], st);
    TipoDado t2 = obter_tipo_expressao(node->filhos[1], st);

    // Operadores relacionais e lógicos
    if (strcmp(node->op, "==") == 0 || strcmp(node->op, "!=") == 0 ||
        strcmp(node->op, "<") == 0 || strcmp(node->op, ">") == 0 ||
        strcmp(node->op, "<=") == 0 || strcmp(node->op, ">=") == 0 ||
        strcmp(node->op, "&&") == 0 || strcmp(node->op, "||") == 0) {
        
        if (!tipo_dado_eh_numerico(t1) || !tipo_dado_eh_numerico(t2)) {
            fprintf(stderr, "Erro Semântico (Linha %d): Operandos incompatíveis para operador '%s'.\n", node->linha, node->op);
            semantic_errors++;
        }
        return TIPO_INT;
    }

    // Operador resto da divisão (%)
    if (strcmp(node->op, "%") == 0) {
        if ((t1 != TIPO_INT && t1 != TIPO_CHAR) || (t2 != TIPO_INT && t2 != TIPO_CHAR)) {
            fprintf(stderr, "Erro Semântico (Linha %d): Operador '%%' requer operandos inteiros.\n", node->linha);
            semantic_errors++;
        }
        return TIPO_INT;
    }

    // Operadores aritméticos comuns
    if (!tipo_dado_eh_numerico(t1) || !tipo_dado_eh_numerico(t2)) {
        fprintf(stderr, "Erro Semântico (Linha %d): Tipos incompatíveis para operação '%s' (%s e %s).\n",
                node->linha, node->op, tipo_dado_to_string(t1), tipo_dado_to_string(t2));
        semantic_errors++;
        return TIPO_VOID;
    }

    if (t1 == TIPO_FLOAT || t2 == TIPO_FLOAT) {
        return TIPO_FLOAT;
    }
    return TIPO_INT;
}

/**
 * Resolve e valida o tipo de operações unárias (ex: -, !).
 */
static TipoDado resolver_op_unario(ASTNode *node, SymbolTable *st) {
    if (node->n_filhos < 1) return TIPO_VOID;
    TipoDado t = obter_tipo_expressao(node->filhos[0], st);
    
    if (strcmp(node->op, "-") == 0) {
        if (!tipo_dado_eh_numerico(t)) {
            fprintf(stderr, "Erro Semântico (Linha %d): Operador '-' requer operando numérico.\n", node->linha);
            semantic_errors++;
        }
        return t;
    }
    if (strcmp(node->op, "!") == 0) {
        if (!tipo_dado_eh_numerico(t)) {
            fprintf(stderr, "Erro Semântico (Linha %d): Operador '!' requer operando numérico/booleano.\n", node->linha);
            semantic_errors++;
        }
        return TIPO_INT;
    }
    return t;
}

/**
 * Resolve e valida expressões pós-fixas (ex: x++, y--). Garante que atuam sobre um lvalue.
 */
static TipoDado resolver_pos_fixo(ASTNode *node, SymbolTable *st) {
    if (node->n_filhos < 1) return TIPO_VOID;
    TipoDado t = obter_tipo_expressao(node->filhos[0], st);
    
    if (!tipo_dado_eh_numerico(t)) {
        fprintf(stderr, "Erro Semântico (Linha %d): Operador '%s' requer operando numérico.\n", node->linha, node->op);
        semantic_errors++;
    }
    
    ASTNode *target = node->filhos[0];
    if (target->type != NODE_IDENTIFICADOR && target->type != NODE_ACESSO_ARRAY && target->type != NODE_ACESSO_MEMBRO) {
        fprintf(stderr, "Erro Semântico (Linha %d): Operador '%s' requer um lvalue.\n", node->linha, node->op);
        semantic_errors++;
    }
    return t;
}

/**
 * Resolve e valida o acesso a elementos de um array (ex: arr[i]).
 */
static TipoDado resolver_acesso_array(ASTNode *node, SymbolTable *st) {
    if (node->n_filhos < 2) return TIPO_VOID;
    TipoDado t_arr = obter_tipo_expressao(node->filhos[0], st);
    TipoDado t_idx = obter_tipo_expressao(node->filhos[1], st);
    
    if (t_idx != TIPO_INT && t_idx != TIPO_CHAR) {
        fprintf(stderr, "Erro Semântico (Linha %d): Índice de array deve ser inteiro.\n", node->linha);
        semantic_errors++;
    }
    return t_arr;
}

/**
 * Resolve e valida o acesso a membros de uma struct (ex: p.x).
 */
static TipoDado resolver_acesso_membro(ASTNode *node, SymbolTable *st) {
    if (node->n_filhos < 1) return TIPO_VOID;
    ASTNode *obj_node = node->filhos[0];
    
    Symbol *sym_obj = NULL;
    if (obj_node->type == NODE_IDENTIFICADOR) {
        sym_obj = lookup_symbol(st, obj_node->valor);
    }
    
    if (!sym_obj) {
        if (obj_node->type == NODE_IDENTIFICADOR) {
            fprintf(stderr, "Erro Semântico (Linha %d): Objeto '%s' não declarado.\n", node->linha, obj_node->valor);
            semantic_errors++;
        }
        return TIPO_VOID;
    }
    
    if (sym_obj->escopo_membros == NULL) {
        fprintf(stderr, "Erro Semântico (Linha %d): Identificador '%s' não é uma estrutura (struct).\n", node->linha, sym_obj->lexeme);
        semantic_errors++;
        return TIPO_VOID;
    }
    
    Symbol *member = lookup_symbol(sym_obj->escopo_membros, node->valor);
    if (!member) {
        fprintf(stderr, "Erro Semântico (Linha %d): Membro '%s' não existe na estrutura '%s'.\n", node->linha, node->valor, sym_obj->lexeme);
        semantic_errors++;
        return TIPO_VOID;
    }
    
    return member->tipo_dado;
}

/**
 * Resolve e valida chamadas a funções, aferindo número e compatibilidade dos parâmetros.
 */
static TipoDado resolver_chamada_func(ASTNode *node, SymbolTable *st) {
    if (node->n_filhos < 1) return TIPO_VOID;
    ASTNode *func_id = node->filhos[0];
    if (func_id->type != NODE_IDENTIFICADOR) return TIPO_VOID;
    
    Symbol *sym = lookup_symbol(st, func_id->valor);
    if (!sym) {
        fprintf(stderr, "Erro Semântico (Linha %d): Função '%s' não declarada.\n", node->linha, func_id->valor);
        semantic_errors++;
        return TIPO_VOID;
    }
    if (sym->categoria != CAT_METODO) {
        fprintf(stderr, "Erro Semântico (Linha %d): Identificador '%s' não é uma função.\n", node->linha, func_id->valor);
        semantic_errors++;
        return TIPO_VOID;
    }
    
    ASTNode *args_list = (node->n_filhos > 1) ? node->filhos[1] : NULL;
    ParamNode *param = sym->parametros;
    int arg_idx = 1;
    
    if (args_list && args_list->type == NODE_LISTA_DECL_GLOBAIS) {
        for (int i = 0; i < args_list->n_filhos; i++) {
            ASTNode *arg = args_list->filhos[i];
            TipoDado arg_type = obter_tipo_expressao(arg, st);
            
            if (!param) {
                fprintf(stderr, "Erro Semântico (Linha %d): Demasiados argumentos na chamada de '%s'.\n", node->linha, sym->lexeme);
                semantic_errors++;
                break;
            }
            
            if (!tipos_compativeis(param->tipo, arg_type)) {
                fprintf(stderr, "Erro Semântico (Linha %d): Argumento %d de '%s' incompatível. Esperado %s, obtido %s.\n",
                        node->linha, arg_idx, sym->lexeme, tipo_dado_to_string(param->tipo), tipo_dado_to_string(arg_type));
                semantic_errors++;
            }
            
            param = param->next;
            arg_idx++;
        }
    }
    
    if (param) {
        fprintf(stderr, "Erro Semântico (Linha %d): Faltam argumentos na chamada de '%s'.\n", node->linha, sym->lexeme);
        semantic_errors++;
    }
    
    return sym->retorno;
}

/**
 * Resolve e valida expressões de atribuição (=). Garante atribuições a lvalues compatíveis.
 */
static TipoDado resolver_atribuicao(ASTNode *node, SymbolTable *st) {
    if (node->n_filhos < 2) return TIPO_VOID;
    TipoDado t_dest = obter_tipo_expressao(node->filhos[0], st);
    TipoDado t_src = obter_tipo_expressao(node->filhos[1], st);
    
    ASTNode *dest_node = node->filhos[0];
    if (dest_node->type != NODE_IDENTIFICADOR && dest_node->type != NODE_ACESSO_ARRAY && dest_node->type != NODE_ACESSO_MEMBRO) {
        fprintf(stderr, "Erro Semântico (Linha %d): Destino da atribuição deve ser um lvalue.\n", node->linha);
        semantic_errors++;
    }
    
    if (!tipos_compativeis(t_dest, t_src)) {
        fprintf(stderr, "Erro Semântico (Linha %d): Tipo incompatível na atribuição. Impossível atribuir tipo %s a %s.\n",
                node->linha, tipo_dado_to_string(t_src), tipo_dado_to_string(t_dest));
        semantic_errors++;
    }
    return t_dest;
}

/* =========================================================================
   IMPLEMENTAÇÃO DAS FUNÇÕES AUXILIARES DE DECLARAÇÕES E ESTRUTURAS (STATIC)
   ========================================================================= */

/**
 * Processa a semântica de uma declaração de variável local, global ou atributo de struct.
 */
static void analisar_decl_variavel(ASTNode *node, SymbolTable *st, EscopoSimbolo escopo_atual) {
    Symbol *sym = (Symbol *)calloc(1, sizeof(Symbol));
    if (!sym) return;
    
    sym->lexeme = strdup(node->valor ? node->valor : "");
    sym->token = strdup("IDENTIFIER");
    sym->linha = node->linha;
    sym->categoria = CAT_VARIAVEL;
    sym->escopo = escopo_atual;
    sym->qtd_dimensoes = node->dimensions;
    
    if (escopo_atual == ESCOPO_CLASSE) {
        sym->tipo_var = VAR_ATRIBUTO;
    } else if (escopo_atual == ESCOPO_METODO) {
        sym->tipo_var = VAR_LOCAL;
    } else {
        sym->tipo_var = VAR_GLOBAL;
    }

    if (node->n_filhos > 0 && node->filhos[0]->type == NODE_ESPECIFICADOR_TIPO) {
        ASTNode *tipo_node = node->filhos[0];
        sym->tipo_dado = mapear_tipo(tipo_node->valor, st);
        
        if (sym->tipo_dado == TIPO_STRUCT) {
            sym->tipo_customizado = strdup(tipo_node->valor);
            Symbol *type_sym = lookup_symbol(st, tipo_node->valor);
            if (type_sym && type_sym->categoria == CAT_CLASSE) {
                sym->escopo_membros = type_sym->escopo_membros;
            }
        }
        
        if (sym->tipo_dado == TIPO_INT || sym->tipo_dado == TIPO_FLOAT) {
            sym->tamanho = 4;
        } else if (sym->tipo_dado == TIPO_CHAR) {
            sym->tamanho = 1;
        }
    }

    if (node->n_filhos > 1) {
        sym->inicializado = true;
        if (node->filhos[1]->valor) {
            sym->valor = strdup(node->filhos[1]->valor);
        }
        TipoDado t_init = obter_tipo_expressao(node->filhos[1], st);
        if (!tipos_compativeis(sym->tipo_dado, t_init)) {
            fprintf(stderr, "Erro Semântico (Linha %d): Inicialização de tipo incompatível para variável '%s'. Esperado %s, obtido %s.\n",
                    node->linha, sym->lexeme, tipo_dado_to_string(sym->tipo_dado), tipo_dado_to_string(t_init));
            semantic_errors++;
        }
    } else {
        sym->inicializado = false;
    }

    if (!insert_symbol(st, sym)) {
        fprintf(stderr, "Erro Semântico (Linha %d): Variável '%s' já declarada neste escopo.\n", sym->linha, sym->lexeme);
        semantic_errors++;
        free(sym->token); free(sym->lexeme); if (sym->valor) free(sym->valor); if (sym->tipo_customizado) free(sym->tipo_customizado); free(sym);
    }
}

/**
 * Processa a semântica de uma declaração typedef (estruturação de um novo tipo / struct).
 */
static void analisar_decl_typedef(ASTNode *node, SymbolTable *st) {
    Symbol *sym = (Symbol *)calloc(1, sizeof(Symbol));
    if (!sym) return;
    sym->lexeme = strdup(node->valor ? node->valor : "struct_anonima");
    sym->token = strdup("IDENTIFIER");
    sym->linha = node->linha;
    sym->categoria = CAT_CLASSE;
    sym->escopo = ESCOPO_GLOBAL;
    sym->tipo_dado = TIPO_STRUCT;

    SymbolTable *escopo_struct = NULL;
    if (node->n_filhos > 0 && node->filhos[0]->type == NODE_ESPECIFICADOR_TIPO) {
        ASTNode *struct_spec = node->filhos[0];
        if (struct_spec->valor && strcmp(struct_spec->valor, "struct") == 0) {
            escopo_struct = create_table(st);
            for (int i = 0; i < struct_spec->n_filhos; i++) {
                analisar_semantica(struct_spec->filhos[i], escopo_struct, ESCOPO_CLASSE);
            }
        }
    }
    
    sym->escopo_membros = escopo_struct;

    if (!insert_symbol(st, sym)) {
        fprintf(stderr, "Erro Semântico (Linha %d): Tipo '%s' já declarado.\n", sym->linha, sym->lexeme);
        semantic_errors++;
        if (escopo_struct) free_table(escopo_struct);
        free(sym->token); free(sym->lexeme); free(sym);
        return;
    }
}

/**
 * Processa a semântica de uma declaração de função e inicializa a análise do seu bloco interno.
 */
static void analisar_decl_funcao(ASTNode *node, SymbolTable *st, EscopoSimbolo escopo_atual) {
    Symbol *sym = (Symbol *)calloc(1, sizeof(Symbol));
    if (!sym) return;
    sym->lexeme = strdup(node->valor ? node->valor : "");
    sym->token = strdup("IDENTIFIER");
    sym->linha = node->linha;
    sym->categoria = CAT_METODO;
    sym->escopo = escopo_atual;

    if (node->n_filhos > 0 && node->filhos[0]->type == NODE_ESPECIFICADOR_TIPO) {
        sym->retorno = mapear_tipo(node->filhos[0]->valor, st);
    }

    extrair_parametros(node, sym);

    if (!insert_symbol(st, sym)) {
        fprintf(stderr, "Erro Semântico (Linha %d): Função '%s' já definida.\n", sym->linha, sym->lexeme);
        semantic_errors++;
        ParamNode *p = sym->parametros;
        while (p) {
            ParamNode *tmp = p;
            p = p->next;
            free(tmp->nome); free(tmp);
        }
        free(sym->token); free(sym->lexeme); free(sym);
        return;
    }

    SymbolTable *escopo_local = create_table(st);
    
    TipoDado retorno_anterior = esperado_retorno;
    esperado_retorno = sym->retorno;

    ParamNode *p = sym->parametros;
    while (p) {
        Symbol *psym = (Symbol *)calloc(1, sizeof(Symbol));
        if (psym) {
            psym->lexeme = strdup(p->nome);
            psym->token = strdup("IDENTIFIER");
            psym->linha = node->linha;
            psym->categoria = CAT_PARAMETRO;
            psym->escopo = ESCOPO_METODO;
            psym->tipo_var = VAR_LOCAL;
            psym->tipo_dado = p->tipo;
            psym->inicializado = true;
            insert_symbol(escopo_local, psym);
        }
        p = p->next;
    }
    
    for (int i = 0; i < node->n_filhos; i++) {
        if (node->filhos[i]->type != NODE_ESPECIFICADOR_TIPO && node->filhos[i]->type != NODE_DECLARACAO_VARIAVEL) {
            analisar_semantica(node->filhos[i], escopo_local, ESCOPO_METODO);
        }
    }
    
    esperado_retorno = retorno_anterior;
    free_table(escopo_local);
}

/**
 * Processa a semântica de uma estrutura condicional 'if'.
 */
static void analisar_if(ASTNode *node, SymbolTable *st, EscopoSimbolo escopo_atual) {
    if (node->n_filhos > 0) {
        TipoDado t_cond = obter_tipo_expressao(node->filhos[0], st);
        if (!tipo_dado_eh_numerico(t_cond)) {
            fprintf(stderr, "Erro Semântico (Linha %d): Condição do 'if' deve ser de tipo numérico/booleano.\n", node->linha);
            semantic_errors++;
        }
    }
    for (int i = 1; i < node->n_filhos; i++) {
        analisar_semantica(node->filhos[i], st, escopo_atual);
    }
}

/**
 * Processa a semântica de estruturas de repetição 'while' e 'do-while'.
 */
static void analisar_while_do_while(ASTNode *node, SymbolTable *st, EscopoSimbolo escopo_atual) {
    int cond_idx = (node->type == NODE_DO_WHILE) ? 1 : 0;
    int body_idx = (node->type == NODE_DO_WHILE) ? 0 : 1;
    
    if (node->n_filhos > cond_idx) {
        TipoDado t_cond = obter_tipo_expressao(node->filhos[cond_idx], st);
        if (!tipo_dado_eh_numerico(t_cond)) {
            fprintf(stderr, "Erro Semântico (Linha %d): Condição do laço deve ser de tipo numérico/booleano.\n", node->linha);
            semantic_errors++;
        }
    }
    if (node->n_filhos > body_idx) {
        analisar_semantica(node->filhos[body_idx], st, escopo_atual);
    }
}

/**
 * Processa a semântica de uma estrutura de repetição 'for'.
 */
static void analisar_for(ASTNode *node, SymbolTable *st, EscopoSimbolo escopo_atual) {
    if (node->n_filhos == 4) {
        obter_tipo_expressao(node->filhos[0], st);
        
        if (node->filhos[1]->type != NODE_VAZIO) {
            TipoDado t_cond = obter_tipo_expressao(node->filhos[1], st);
            if (!tipo_dado_eh_numerico(t_cond)) {
                fprintf(stderr, "Erro Semântico (Linha %d): Condição do 'for' deve ser de tipo numérico/booleano.\n", node->linha);
                semantic_errors++;
            }
        }
        
        obter_tipo_expressao(node->filhos[2], st);
        analisar_semantica(node->filhos[3], st, escopo_atual);
    }
}

/**
 * Processa a semântica de uma instrução 'return', aferindo compatibilidade com o retorno do método correspondente.
 */
static void analisar_return(ASTNode *node, SymbolTable *st) {
    if (node->n_filhos > 0) {
        TipoDado t_ret = obter_tipo_expressao(node->filhos[0], st);
        if (!tipos_compativeis(esperado_retorno, t_ret)) {
            fprintf(stderr, "Erro Semântico (Linha %d): Retorno incompatível. A função espera tipo %s, mas obteve %s.\n",
                    node->linha, tipo_dado_to_string(esperado_retorno), tipo_dado_to_string(t_ret));
            semantic_errors++;
        }
    } else {
        if (esperado_retorno != TIPO_VOID) {
            fprintf(stderr, "Erro Semântico (Linha %d): Expressão de retorno em falta (esperado tipo %s).\n",
                    node->linha, tipo_dado_to_string(esperado_retorno));
            semantic_errors++;
        }
    }
}

/* =========================================================================
   OUTRAS FUNÇÕES AUXILIARES DE SUPORTE (STATIC)
   ========================================================================= */

/**
 * Converte o lexema de tipo da AST (ex: "int", "struct") para o TipoDado enum correspondente.
 */
static TipoDado mapear_tipo(const char *tipo_str, SymbolTable *st) {
    if (!tipo_str) return TIPO_VOID;
    if (strcmp(tipo_str, "int") == 0) return TIPO_INT;
    if (strcmp(tipo_str, "float") == 0) return TIPO_FLOAT;
    if (strcmp(tipo_str, "char") == 0) return TIPO_CHAR;
    if (strcmp(tipo_str, "void") == 0) return TIPO_VOID;
    if (strcmp(tipo_str, "struct") == 0) return TIPO_STRUCT;
    
    if (st) {
        Symbol *sym = lookup_symbol(st, tipo_str);
        if (sym && sym->categoria == CAT_CLASSE) {
            return TIPO_STRUCT;
        }
    }
    return TIPO_VOID;
}