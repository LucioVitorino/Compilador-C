#include "parser.h"

static void recover_declaracao(Parser *p, const char *msg)
{
    syntax_error_recover(p, msg, SYNC_DECLARACAO);
}

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
                if (!parser_consume_if_type(p, "TOK_GT"))
                    recover_declaracao(p, "Falta o token '>' no include do sistema");
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
                if (!parser_consume_if_type(p, "TOK_DQUOTE"))
                    recover_declaracao(p, "Falta a aspas de fecho no include local");
            }
            else
            {
                recover_declaracao(p, "Esperado '<' ou aspas duplas após '#include'");
            }
            return n;
        }
        recover_declaracao(p, "Diretiva de preprocessador inválida");
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
        if (!tipo)
        {
            recover_declaracao(p, "Esperado um tipo valido após 'typedef'");
            return NULL;
        }
        parse_asteriscos(p);
        if (p->current && p->current->type && strcmp(p->current->type, "IDENTIFIER") == 0)
        {
            const char *name = p->current->value ? p->current->value : "";
            int line = p->current->line;
            parser_next_token(p);
            // skip optional array and semicolon
            parse_sufixo_array_opcional(p);
            if (!parser_consume_if_value(p, ";"))
                recover_declaracao(p, "Esperado ';' no final do typedef");
            ASTNode *n = make_node(NODE_DECLARACAO_TYPEDEF, line);
            n->valor = strdup(name);
            if (tipo)
                add_filho(n, tipo);
            return n;
        }
        recover_declaracao(p, "Esperado o nome do alias no typedef");
        return NULL;
    }

    /* general declaration: type ... */
    ASTNode *tipo = parse_especificador_tipo(p);
    if (!tipo)
    {
        recover_declaracao(p, "Esperado um especificador de tipo na declaracao global");
        return NULL;
    }
    // pointers
    int stars = parse_asteriscos(p);
    (void)stars;
    if (!p->current)
    {
        recover_declaracao(p, "Declaracao global incompleta");
        return NULL;
    }
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
            if (!parser_consume_if_value(p, ")"))
                recover_declaracao(p, "Falta o parêntese de fecho na declaracao da funcao");

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
                else
                    recover_declaracao(p, "Esperado corpo de funcao ou ';' no prototipo");
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
                else
                    recover_declaracao(p, "Expressão inválida na inicialização da variável global");
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
                        else
                            recover_declaracao(p, "Expressão inválida na inicialização de declarador global");
                    }
                    add_filho(decl_list, v);
                }
                else
                {
                    // skip junk until ;
                    while (p->current && p->current->value && strcmp(p->current->value, ";") != 0)
                        parser_next_token(p);
                    recover_declaracao(p, "Esperado identificador após ',' na declaracao global");
                    break;
                }
            }
            if (!parser_consume_if_value(p, ";"))
                recover_declaracao(p, "Esperado ';' no final da declaracao global");
            return decl_list;
        }
    }
    if (p->current) {
        syntax_error_recover(p, "Esperado um especificador de tipo ou diretiva na declaracao global", SYNC_DECLARACAO);
        return NULL;
    }
    return NULL;
}
