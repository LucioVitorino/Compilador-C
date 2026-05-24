#include "parser.h"

ASTNode *parse_declaracao_global(Parser *p)
{
    // preprocessor directive: new tokenized form TOK_HASH TOK_INCLUDE ...
    if (p->current && p->current->type && strcmp(p->current->type, "TOK_HASH") == 0)
    {
        int line = p->current->line;
        parser_next_token(p); // consume '#'
        if (p->current && p->current->type && strcmp(p->current->type, "TOK_INCLUDE") == 0)
        {
            parser_next_token(p); // consume 'include'
            ASTNode *n = make_node(NODE_DIRETIVA_INCLUDE, line);
            // either < filename > or " filename "
            if (p->current && p->current->type && strcmp(p->current->type, "TOK_LT") == 0)
            {
                parser_next_token(p);
                if (p->current && p->current->type && strcmp(p->current->type, "TOK_NOME_FICHEIRO") == 0)
                {
                    ASTNode *fname = make_folha(NODE_NOME_FICHEIRO, p->current->value, p->current->line);
                    fname->op = strdup("system");
                    add_filho(n, fname);
                    parser_next_token(p);
                }
                parser_consume_if_type(p, "TOK_GT");
            }
            else if (p->current && p->current->type && strcmp(p->current->type, "TOK_DQUOTE") == 0)
            {
                parser_next_token(p);
                if (p->current && p->current->type && strcmp(p->current->type, "TOK_NOME_FICHEIRO") == 0)
                {
                    ASTNode *fname = make_folha(NODE_NOME_FICHEIRO, p->current->value, p->current->line);
                    fname->op = strdup("local");
                    add_filho(n, fname);
                    parser_next_token(p);
                }
                parser_consume_if_type(p, "TOK_DQUOTE");
            }
            return n;
        }
        // if not include, fall through and let other directive handling happen
    }

    // legacy: preprocessor directive captured as single token
    if (p->current && p->current->type && strcmp(p->current->type, "PREPROCESSOR_DIRECTIVE") == 0)
    {
        const char *val = p->current->value ? p->current->value : "";
        int line = p->current->line;
        parser_next_token(p);
        ASTNode *n = make_node(NODE_DIRETIVA_INCLUDE, line);
        n->valor = strdup(val);
        return n;
    }

    // typedef
    if (p->current && p->current->type && strcmp(p->current->type, "KEYWORD_TYPEDEF") == 0)
    {
        parser_next_token(p);
        ASTNode *tipo = parse_especificador_tipo(p);
        parse_asteriscos(p);
        if (p->current && p->current->type && strcmp(p->current->type, "IDENTIFIER") == 0)
        {
            const char *name = p->current->value ? p->current->value : "";
            int line = p->current->line;
            parser_next_token(p);
            // skip optional array and semicolon
            parse_sufixo_array_opcional(p);
            parser_consume_if_value(p, ";");
            ASTNode *n = make_node(NODE_DECLARACAO_TYPEDEF, line);
            n->valor = strdup(name);
            if (tipo)
                add_filho(n, tipo);
            return n;
        }
    }

    /* general declaration: type ... */
    ASTNode *tipo = parse_especificador_tipo(p);
    if (!tipo)
        return NULL;
    // pointers
    int stars = parse_asteriscos(p);
    (void)stars;
    if (!p->current)
        return NULL;
    if (p->current->type && strcmp(p->current->type, "IDENTIFIER") == 0)
    {
        const char *name = p->current->value ? p->current->value : "";
        int line = p->current->line;
        parser_next_token(p);
        // function? '(' => parse params and block
        // Em declaracao_global.c — Alinhando com <cauda_declaracao_geral> e <corpo_ou_ponto_virgula>
        if (p->current && p->current->value && strcmp(p->current->value, "(") == 0)
        {
            parser_next_token(p); // consome '('
            parse_parametros_opcionais(p);
            parser_consume_if_value(p, ")");

            ASTNode *func = make_node(NODE_DECLARACAO_FUNCAO, line);
            func->valor = strdup(name);
            add_filho(func, tipo);

            // IMPLEMENTAÇÃO DA NOVA REGRA <corpo_ou_ponto_virgula>
            if (p->current && p->current->value && strcmp(p->current->value, ";") == 0)
            {
                parser_next_token(p); // É um protótipo de função, consome ';' e fecha o nó
                // Podes adicionar uma flag ou folha indicando que é apenas uma assinatura
                ASTNode *proto = make_folha(NODE_IDENTIFICADOR, "proto", p->current ? p->current->line : line);
                add_filho(func, proto);
            }
            else
            {
                ASTNode *body = parse_bloco(p); // É a definição completa, processa o bloco {}
                if (body)
                    add_filho(func, body);
            }
            return func;
        }
        else
        {
            // first declarator
            parse_sufixo_array_opcional(p);
            ASTNode *var = make_node(NODE_DECLARACAO_VARIAVEL, line);
            var->valor = strdup(name);
            add_filho(var, tipo);
            // inicialização
            if (p->current && p->current->value && strcmp(p->current->value, "=") == 0)
            {
                parser_next_token(p);
                ASTNode *expr = parse_expressao(p);
                if (expr)
                    add_filho(var, expr);
            }
            // lista de variáveis globais
            ASTNode *decl_list = make_node(NODE_LISTA_DECL_GLOBAIS, line);
            add_filho(decl_list, var);
            // more declarators separated by commas
            while (p->current && p->current->value && strcmp(p->current->value, ",") == 0)
            {
                parser_next_token(p); // consume ','
                parse_asteriscos(p);
                if (p->current && p->current->type && strcmp(p->current->type, "IDENTIFIER") == 0)
                {
                    const char *nname = p->current->value ? p->current->value : "";
                    int nline = p->current->line;
                    parser_next_token(p);
                    parse_sufixo_array_opcional(p);
                    ASTNode *v = make_node(NODE_DECLARACAO_VARIAVEL, nline);
                    v->valor = strdup(nname);
                    add_filho(v, tipo);
                    if (p->current && p->current->value && strcmp(p->current->value, "=") == 0)
                    {
                        parser_next_token(p);
                        ASTNode *expr = parse_expressao(p);
                        if (expr)
                            add_filho(v, expr);
                    }
                    add_filho(decl_list, v);
                }
                else
                {
                    // skip junk until ;
                    while (p->current && p->current->value && strcmp(p->current->value, ";") != 0)
                        parser_next_token(p);
                    break;
                }
            }
            parser_consume_if_value(p, ";");
            return decl_list;
        }
    }
    return NULL;
}
