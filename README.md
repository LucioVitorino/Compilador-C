# Compilador-C

## Descrição
Este projecto é um front-end de compilador em C, organizado em fases: lexer, parser, AST e análise semântica. O objectivo é ler um ficheiro fonte em C, transformar o texto em tokens, construir uma árvore sintática e validar regras semânticas sobre os símbolos encontrados.

O projecto foi desenvolvido como uma base educativa e de estudo para compreender os conceitos clássicos de compilação: autómatos, gramáticas, parsing descendente, AST, tabela de símbolos e recuperação de erro.

## Estrutura do projecto
- [Lexer/](Lexer) — reconhecimento de tokens e lexemas.
- [parser/](parser) — análise sintática, recuperação de erro e construção da AST.
- [ast/](ast) — estrutura da árvore sintática.
- [semantico/](semantico) — tabela de símbolos e análise semântica.
- [utils/](utils) — funções auxiliares reutilizáveis.
- [testes/](testes) — exemplos de código para validação.
- [Manual_do_Programador.md](Manual_do_Programador.md) — guia técnico para manutenção e evolução do projecto.

## Fases implementadas
- Analisador Léxico: reconhecimento de identificadores, palavras-chave, literais, operadores, delimitadores e diretivas.
- Parser: construção da AST através de uma abordagem descendente recursiva.
- AST: representação hierárquica dos nós sintáticos.
- Análise Semântica: construção e consulta da tabela de símbolos, com suporte a escopos, tipos, parâmetros e informação de símbolos.

## Como compilar
No WSL, a compilação pode ser feita com o comando abaixo:

```bash
wsl bash -lc "cd /mnt/c/Users/eliza/lucio/Compilador-C && gcc -Wall -Wextra -g -I. -ILexer -Iparser -Iast -Iutils -Isemantico -o compilador main.c Lexer/lexer.c Lexer/idenumKey.c Lexer/operators.c Lexer/stringchar.c Lexer/numeric.c Lexer/processor_directive.c utils/get_next_line.c utils/ft_strjoin.c utils/token_operating.c ast/ast.c parser/init.c parser/panic_mode.c parser/expressao.c parser/tipo.c parser/helpers.c parser/declaracao_global.c parser/programa.c parser/statements.c semantico/tabela_simbolos.c semantico/analisador_semantico.c"
```

## Como executar
Depois da compilação, executar o programa com um ficheiro de teste:

```bash
./compilador testes/teste_identificadores.c
```

## Funcionalidades principais
- Leitura de código fonte a partir de um ficheiro.
- Geração de tokens com linha e tipo.
- Construção de uma árvore sintática.
- Recuperação de erros sintáticos em modo pânico.
- Registo de símbolos em tabela de símbolos.
- Suporte a estrutura para evolução da análise semântica.

## Documentação
- [Manual_do_Programador.md](Manual_do_Programador.md) — documentação técnica do projecto.
- [gramática_C.md](gramática_C.md) — descrição da gramática usada como base.
- [parser_mapa.md](parser_mapa.md) — mapa do fluxo do parser e da AST.

## Notas úteis
- O projecto foi validado no WSL com compilação directa via `gcc`.
- O `make` não está disponível na distribuição WSL usada, por isso a compilação é feita de forma explícita.
- Para depuração, o código inclui pontos de impressão da AST e da tabela de símbolos.

