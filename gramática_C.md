<programa> ::= <lista_declaracoes_globais> TOKEN_EOF

<lista_declaracoes_globais> ::= <declaracao_global> <lista_declaracoes_globais>
                              | ε

<declaracao_global> ::= <diretiva_include>
                      | <diretiva_define>
                      | <declaracao_typedef>
                      | <declaracao_geral>

<prefixo_include> ::= TOKEN_HASH TOKEN_INCLUDE
<prefixo_define> ::= TOKEN_HASH TOKEN_DEFINE

<diretiva_include> ::= <prefixo_include> TOKEN_LT <nome_ficheiro> TOKEN_GT
                     | <prefixo_include> TOKEN_STRING_LITERAL

<nome_ficheiro> ::= TOKEN_IDENTIFIER <resto_nome_ficheiro>

<resto_nome_ficheiro> ::= TOKEN_SLASH TOKEN_IDENTIFIER <resto_nome_ficheiro>
                        | TOKEN_DOT <extensao>

<extensao> ::= TOKEN_IDENTIFIER

<diretiva_define> ::= <prefixo_define> TOKEN_IDENTIFIER <resto_define>

<resto_define> ::= <valor_define>
                 | ε

<valor_define> ::= TOKEN_INT_LITERAL
                 | TOKEN_FLOAT_LITERAL
                 | TOKEN_STRING_LITERAL
                 | TOKEN_IDENTIFIER

<especificador_tipo> ::= TOKEN_INT
                       | TOKEN_FLOAT
                       | TOKEN_CHAR
                       | TOKEN_VOID
                       | TOKEN_STRUCT <nome_ou_corpo_struct>
                       | TOKEN_IDENTIFIER

<nome_ou_corpo_struct> ::= TOKEN_IDENTIFIER <corpo_campos_opcional>
                         | <corpo_campos>

<corpo_campos_opcional> ::= <corpo_campos>
                          | ε

<corpo_campos> ::= TOKEN_LBRACE <lista_campos> TOKEN_RBRACE

<lista_campos> ::= <campo> <resto_lista_campos>

<resto_lista_campos> ::= <lista_campos>
                       | ε

<campo> ::= <especificador_tipo> <lista_declaradores_campo> TOKEN_SEMICOLON

<lista_declaradores_campo> ::= <asteriscos> TOKEN_IDENTIFIER <sufixo_array_opcional> <mais_declaradores_campo>

<mais_declaradores_campo> ::= TOKEN_COMMA <asteriscos> TOKEN_IDENTIFIER <sufixo_array_opcional> <mais_declaradores_campo>
                            | ε

<declaracao_typedef> ::= TOKEN_TYPEDEF <especificador_tipo> <asteriscos> TOKEN_IDENTIFIER <sufixo_array_opcional> TOKEN_SEMICOLON

<declaracao_geral> ::= <especificador_tipo> <resto_declaracao_geral>

<resto_declaracao_geral> ::= TOKEN_SEMICOLON
                           | <asteriscos> TOKEN_IDENTIFIER <cauda_declaracao_geral>

<cauda_declaracao_geral> ::= TOKEN_LPAREN <parametros_opcionais> TOKEN_RPAREN <corpo_ou_ponto_virgula>
                           | <sufixo_array_opcional> <inicializacao_opcional> <mais_declaradores> TOKEN_SEMICOLON

<corpo_ou_ponto_virgula> ::= <bloco>
                           | TOKEN_SEMICOLON

<asteriscos> ::= TOKEN_STAR <asteriscos>
               | ε

<sufixo_array_opcional> ::= TOKEN_LBRACKET <expressao> TOKEN_RBRACKET <sufixo_array_opcional>
                          | ε

<inicializacao_opcional> ::= TOKEN_ASSIGN <expressao>
                           | ε

<mais_declaradores> ::= TOKEN_COMMA <asteriscos> TOKEN_IDENTIFIER <sufixo_array_opcional> <inicializacao_opcional> <mais_declaradores>
                      | ε

<parametros_opcionais> ::= TOKEN_VOID
                         | <lista_parametros>
                         | ε

<lista_parametros> ::= <parametro> <mais_parametros>

<mais_parametros> ::= TOKEN_COMMA <parametro> <mais_parametros>
                    | ε

<parametro> ::= <especificador_tipo> <resto_parametro>

<resto_parametro> ::= <asteriscos> TOKEN_IDENTIFIER <sufixo_array_opcional>
                    | ε

<bloco> ::= TOKEN_LBRACE <lista_itens_bloco> TOKEN_RBRACE

<lista_itens_bloco> ::= <item_bloco> <lista_itens_bloco>
                      | ε

<item_bloco> ::= <declaracao_variavel_local>
               | <instrucao>

<declaracao_variavel_local> ::= <especificador_tipo> <asteriscos> TOKEN_IDENTIFIER <sufixo_array_opcional> <inicializacao_opcional> <mais_declaradores> TOKEN_SEMICOLON

<instrucao> ::= <instrucao_expressao>
              | <instrucao_if>
              | <instrucao_while>
              | <instrucao_for>
              | <instrucao_do>
              | <instrucao_return>
              | <bloco>
              | TOKEN_SEMICOLON

<instrucao_expressao> ::= <expressao> TOKEN_SEMICOLON

<instrucao_if> ::= TOKEN_IF TOKEN_LPAREN <expressao> TOKEN_RPAREN <instrucao> <ramo_else_opcional>

<ramo_else_opcional> ::= TOKEN_ELSE <instrucao>
                       | ε

<instrucao_while> ::= TOKEN_WHILE TOKEN_LPAREN <expressao> TOKEN_RPAREN <instrucao>

<instrucao_do> ::= TOKEN_DO <instrucao> TOKEN_WHILE TOKEN_LPAREN <expressao> TOKEN_RPAREN TOKEN_SEMICOLON

<instrucao_for> ::= TOKEN_FOR TOKEN_LPAREN <expressao_opcional> TOKEN_SEMICOLON <expressao_opcional> TOKEN_SEMICOLON <expressao_opcional> TOKEN_RPAREN <instrucao>

<expressao_opcional> ::= <expressao>
                       | ε

<instrucao_return> ::= TOKEN_RETURN <expressao_opcional> TOKEN_SEMICOLON

<expressao> ::= <atribuicao>

<atribuicao> ::= <logico_ou> <atribuicao_r>

<atribuicao_r> ::= TOKEN_ASSIGN <atribuicao>
                 | TOKEN_PLUS_ASSIGN <atribuicao>
                 | TOKEN_MINUS_ASSIGN <atribuicao>
                 | TOKEN_STAR_ASSIGN <atribuicao>
                 | TOKEN_SLASH_ASSIGN <atribuicao>
                 | TOKEN_PERCENT_ASSIGN <atribuicao>
                 | ε

<logico_ou> ::= <logico_e> <logico_ou_r>

<logico_ou_r> ::= TOKEN_OR <logico_e> <logico_ou_r>
                | ε

<logico_e> ::= <igualdade> <logico_e_r>

<logico_e_r> ::= TOKEN_AND <igualdade> <logico_e_r>
               | ε

<igualdade> ::= <relacional> <igualdade_r>

<igualdade_r> ::= TOKEN_EQ <relacional> <igualdade_r>
                | TOKEN_NEQ <relacional> <igualdade_r>
                | ε

<relacional> ::= <aditivo> <relacional_r>

<relacional_r> ::= TOKEN_LT <aditivo> <relacional_r>
                 | TOKEN_GT <aditivo> <relacional_r>
                 | TOKEN_LEQ <aditivo> <relacional_r>
                 | TOKEN_GEQ <aditivo> <relacional_r>
                 | ε

<aditivo> ::= <multiplicativo> <aditivo_r>

<aditivo_r> ::= TOKEN_PLUS <multiplicativo> <aditivo_r>
              | TOKEN_MINUS <multiplicativo> <aditivo_r>
              | ε

<multiplicativo> ::= <unario> <multiplicativo_r>

<multiplicativo_r> ::= TOKEN_STAR <unario> <multiplicativo_r>
                     | TOKEN_SLASH <unario> <multiplicativo_r>
                     | TOKEN_PERCENT <unario> <multiplicativo_r>
                     | ε

<unario> ::= TOKEN_NOT <unario>
           | TOKEN_MINUS <unario>
           | TOKEN_AMP <unario>
           | TOKEN_STAR <unario>
           | <pos_fixo>

<pos_fixo> ::= <primario> <pos_fixo_r>

<pos_fixo_r> ::= TOKEN_LBRACKET <expressao> TOKEN_RBRACKET <pos_fixo_r>
               | TOKEN_DOT TOKEN_IDENTIFIER <pos_fixo_r>
               | TOKEN_LPAREN <argumentos_opcionais> TOKEN_RPAREN <pos_fixo_r>
               | ε

<primario> ::= TOKEN_IDENTIFIER
             | TOKEN_INT_LITERAL
             | TOKEN_FLOAT_LITERAL
             | TOKEN_CHAR_LITERAL
             | TOKEN_STRING_LITERAL
             | TOKEN_LPAREN <expressao> TOKEN_RPAREN

<argumentos_opcionais> ::= <lista_argumentos>
                         | ε

<lista_argumentos> ::= <expressao> <mais_argumentos>

<mais_argumentos> ::= TOKEN_COMMA <expressao> <mais_argumentos>
                    | ε