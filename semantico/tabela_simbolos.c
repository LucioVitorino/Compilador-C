#include "tabela_simbolos.h"


// Inicializa uma nova tabela apontando para o seu escopo pai
SymbolTable *create_table(SymbolTable *parent) {
    SymbolTable *st = (SymbolTable *)malloc(sizeof(SymbolTable));
    if (!st) return NULL;
    st->parent = parent;
    st->next_endereco = 0;
    for (int i = 0; i < HASH_SIZE; i++) {
        st->table[i] = NULL;
    }
    return st;
}

// Função Hash DJB2 para mapear strings para índices da tabela
unsigned int hash_function(const char *lexeme) {
    unsigned long hash = 5381;
    int c;
    while ((c = *lexeme++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % HASH_SIZE;
}

// Insere um símbolo no escopo ATUAL da tabela
bool insert_symbol(SymbolTable *st, Symbol *sym) {
    if (!st || !sym || !sym->lexeme) return false;
    
    unsigned int idx = hash_function(sym->lexeme);
    
    // Verifica se o símbolo já existe APENAS no escopo atual (Redeclaração inválida)
    Symbol *current = st->table[idx];
    while (current) {
        if (strcmp(current->lexeme, sym->lexeme) == 0) {
            return false; // Erro Semântico: Identificador já declarado neste escopo
        }
        current = current->next;
    }
    
    if (sym->endereco == 0) {
        long tamanho = sym->tamanho > 0 ? sym->tamanho : 1;
        sym->endereco = st->next_endereco;
        st->next_endereco += tamanho;
    }

    // Insere no início da lista encadeada daquela posição (Tratamento de Colisão)
    sym->next = st->table[idx];
    st->table[idx] = sym;
    return true;
}

Symbol *lookup_symbol_current_scope(SymbolTable *st, const char *lexeme) {
    if (!st || !lexeme) return NULL;

    unsigned int idx = hash_function(lexeme);
    Symbol *current = st->table[idx];

    while (current) {
        if (strcmp(current->lexeme, lexeme) == 0) {
            return current;
        }
        current = current->next;
    }

    return NULL;
}

// Procura um símbolo de forma recursiva (Escopo Local -> Escopo Pai -> Global)
Symbol *lookup_symbol(SymbolTable *st, const char *lexeme) {
    if (!st || !lexeme) return NULL;
    
    unsigned int idx = hash_function(lexeme);
    Symbol *current = st->table[idx];
    
    while (current) {
        if (strcmp(current->lexeme, lexeme) == 0) {
            return current; // Encontrado!
        }
        current = current->next;
    }
    
    // Se não encontrou no escopo atual, procura recursivamente no escopo pai
    if (st->parent) {
        return lookup_symbol(st->parent, lexeme);
    }
    
    return NULL; // Símbolo não existe em nenhum escopo visível
}

const char *tipo_dado_to_string(TipoDado tipo) {
    switch (tipo) {
        case TIPO_INT: return "int";
        case TIPO_FLOAT: return "float";
        case TIPO_CHAR: return "char";
        case TIPO_STRING: return "string";
        case TIPO_STRUCT: return "struct";
        case TIPO_VOID:
        default: return "void";
    }
}

bool tipo_dado_eh_numerico(TipoDado tipo) {
    return (tipo == TIPO_INT || tipo == TIPO_FLOAT || tipo == TIPO_CHAR);
}

bool tipos_compativeis(TipoDado destino, TipoDado origem) {
    if (destino == origem) return true;
    if (destino == TIPO_FLOAT && origem == TIPO_INT) return true;
    if (destino == TIPO_INT && origem == TIPO_CHAR) return true;
    return false;
}

// Liberta a memória de uma tabela e de todas as suas entradas
void free_table(SymbolTable *st) {
    if (!st) return;
    for (int i = 0; i < HASH_SIZE; i++) {
        Symbol *current = st->table[i];
        while (current) {
            Symbol *tmp = current;
            current = current->next;
            
            // Liberta strings alocadas dinamicamente
            free(tmp->token);
            free(tmp->lexeme);
            if (tmp->valor) free(tmp->valor);
            if (tmp->modificadores) free(tmp->modificadores);
            
            // Liberta lista de parâmetros se for método
            ParamNode *p = tmp->parametros;
            while (p) {
                ParamNode *ptmp = p;
                p = p->next;
                free(ptmp->nome);
                free(ptmp);
            }
            
            if (tmp->tipo_customizado) free(tmp->tipo_customizado);
            if (tmp->escopo_membros && tmp->categoria == CAT_CLASSE) free_table(tmp->escopo_membros);
            
            free(tmp);
        }
    }
    free(st);
}

// Função utilitária para debug e visualização da tabela no terminal
void print_table(SymbolTable *st) {
    if (!st) return;
    
    printf("\n===================================================================================================\n");
    printf("                                   TABELA DE SÍMBOLS ATUAL                                         \n");
    printf("===================================================================================================\n");
        printf("%-12s | %-12s | %-5s | %-10s | %-8s | %-8s | %-10s | %-8s | %-6s | %-5s | %-6s\n", 
            "Lexema", "Token", "Linha", "Categoria", "Tipo", "Escopo", "Endereco", "Dimensoes", "Tamanho", "Inic.", "Valor");
    printf("---------------------------------------------------------------------------------------------------\n");
    
    for (int i = 0; i < HASH_SIZE; i++) {
        Symbol *s = st->table[i];
        while (s) {
            // 1. Mapear Categoria para texto legível
            char *cat_str = "DESCONHECIDO";
            if (s->categoria == CAT_VARIAVEL)  cat_str = "VARIAVEL";
            else if (s->categoria == CAT_METODO) cat_str = "METODO";
            else if (s->categoria == CAT_CLASSE) cat_str = "CLASSE";
            else if (s->categoria == CAT_PARAMETRO) cat_str = "PARAMETRO";

            // 2. Mapear Tipo de Dado para texto legível
            const char *tipo_str = tipo_dado_to_string(s->tipo_dado);

            // 3. Mapear Escopo para texto legível
            char *esc_str = "GLOBAL";
            if (s->escopo == ESCOPO_METODO) esc_str = "LOCAL";
            else if (s->escopo == ESCOPO_CLASSE) esc_str = "CLASSE";

            // 4. Imprimir a linha formatada com todos os campos da imagem
                 printf("%-12s | %-12s | %-5d | %-10s | %-8s | %-8s | %-10ld | %-8d | %-5d B | %-5s | %-6s\n", 
                   s->lexeme, 
                   s->token, 
                   s->linha, 
                   cat_str, 
                   tipo_str, 
                   esc_str, 
                     s->endereco,
                     s->qtd_dimensoes,
                   s->tamanho, 
                   s->inicializado ? "SIM" : "NAO", 
                   s->valor ? s->valor : "NULL");

            s = s->next;
        }
    }
    printf("===================================================================================================\n\n");
}