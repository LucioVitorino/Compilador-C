#include "parser.h"

ASTNode *parse_especificador_tipo(Parser *p)
{
    if (!p->current) return NULL;
    
    // 1. Tipos primitivos e identificadores (tipos customizados como Ponto)
    if (p->current->type && (strcmp(p->current->type, "KEYWORD_INT") == 0 || 
                             strcmp(p->current->type, "KEYWORD_FLOAT") == 0 || 
                             strcmp(p->current->type, "KEYWORD_CHAR") == 0 || 
                             strcmp(p->current->type, "KEYWORD_VOID") == 0 ||
                             strcmp(p->current->type, "IDENTIFIER") == 0)) {
        const char *val = p->current->value ? p->current->value : "";
        int line = p->current->line;
        parser_next_token(p);
        return make_folha(NODE_ESPECIFICADOR_TIPO, val, line);
    }
    
    // 2. Structs: struct Nome { campos } ou struct { campos }
    if (p->current->type && strcmp(p->current->type, "KEYWORD_STRUCT") == 0) {
        int line = p->current->line;
        parser_next_token(p); // consome 'struct'
        
        ASTNode *struct_node = make_node(NODE_ESPECIFICADOR_TIPO, line);
        struct_node->valor = strdup("struct");
        
        // Nome opcional da struct (ex: struct Ponto)
        if (p->current && p->current->type && strcmp(p->current->type, "IDENTIFIER") == 0) {
            ASTNode *name_node = make_folha(NODE_IDENTIFICADOR, p->current->value, p->current->line);
            add_filho(struct_node, name_node);
            parser_next_token(p); // consome o nome
        }
        
        // Corpo opcional da struct (ex: { int x; int y; })
        if (p->current && p->current->value && strcmp(p->current->value, "{") == 0) {
            parser_next_token(p); // consome '{'
            
            while (p->current && !(p->current->value && strcmp(p->current->value, "}") == 0) && strcmp(p->current->type, "TOK_EOF") != 0) {
                ASTNode *field_type = parse_especificador_tipo(p);
                if (!field_type) {
                    syntax_error_recover(p, "Esperado especificador de tipo no campo da struct", SYNC_DECLARACAO);
                    break;
                }
                
                int stars = parse_asteriscos(p);
                if (p->current && p->current->type && strcmp(p->current->type, "IDENTIFIER") == 0) {
                    const char *f_name = p->current->value ? p->current->value : "";
                    int f_line = p->current->line;
                    parser_next_token(p);
                    int f_dims = parse_sufixo_array_opcional(p);
                    
                    ASTNode *field_var = make_node(NODE_DECLARACAO_VARIAVEL, f_line);
                    field_var->valor = strdup(f_name);
                    field_var->pointer_level = stars;
                    field_var->dimensions = f_dims;
                    add_filho(field_var, field_type);
                    add_filho(struct_node, field_var);
                    
                    // Suporte a declaradores múltiplos (ex: int x, y;)
                    while (p->current && p->current->value && strcmp(p->current->value, ",") == 0) {
                        parser_next_token(p); // consome ','
                        int next_stars = parse_asteriscos(p);
                        if (p->current && p->current->type && strcmp(p->current->type, "IDENTIFIER") == 0) {
                            const char *next_fname = p->current->value;
                            int next_fline = p->current->line;
                            parser_next_token(p);
                            int next_fdims = parse_sufixo_array_opcional(p);
                            
                            ASTNode *next_field = make_node(NODE_DECLARACAO_VARIAVEL, next_fline);
                            next_field->valor = strdup(next_fname);
                            next_field->pointer_level = next_stars;
                            next_field->dimensions = next_fdims;
                            
                            ASTNode *field_type_clone = make_node(NODE_ESPECIFICADOR_TIPO, next_fline);
                            field_type_clone->valor = strdup(field_type->valor ? field_type->valor : "int");
                            add_filho(next_field, field_type_clone);
                            add_filho(struct_node, next_field);
                        } else {
                            syntax_error_recover(p, "Esperado identificador após ',' na struct", SYNC_DECLARACAO);
                            break;
                        }
                    }
                }
                
                parser_expect(p, ";", "Esperado ';' após campo da struct", SYNC_DECLARACAO);
            }
            
            parser_expect(p, "}", "Esperado '}' no fecho da struct", SYNC_DECLARACAO);
        }
        return struct_node;
    }
    
    return NULL;
}
