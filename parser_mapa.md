# Mapa do Parser e da AST

Este documento descreve o fluxo real do parser implementado no projeto, as regras que ele reconhece e como a AST e montada durante a analise sintatica.

## 1. Visao geral do fluxo

O ponto de entrada do processo e `main.c`:

1. O ficheiro fonte e tokenizado por `tokenize(fd)`.
2. A lista de tokens e impressa para depuracao.
3. `parser_init(&p, tokens)` posiciona o parser no primeiro token.
4. `parse_programa(&p)` construi a raiz da AST.
5. `print_ast(ast, 0)` mostra a arvore final.
6. `free_ast(ast)` liberta a memoria da AST.

A implementacao usada pelo build esta dividida em varios modulos:

- `parser/init.c` - navegacao basica de tokens, recuperacao de erro e `parser_expect`.
- `parser/programa.c` - raiz do programa e lista de declaracoes globais.
- `parser/declaracao_global.c` - declaracoes globais, funcoes, typedefs, variaveis e diretivas.
- `parser/statements.c` - blocos e instrucoes.
- `parser/expressao.c` - expressao, precedencia e associatividade.
- `parser/helpers.c` - parametros, ponteiros e sufixos de arrays.
- `parser/tipo.c` - especificador de tipo.

Nota: existe tambem um `parser/parser.c` com trechos de logica duplicada/legada, mas ele nao faz parte do `Makefile` atual. A descricao abaixo segue o caminho realmente compilado.

## 2. Raiz da analise

### `parse_programa`

`parse_programa(Parser *p)` cria um no `NODE_PROGRAMA` e pendura nele a lista de declaracoes globais.

Fluxo:

1. Cria `NODE_PROGRAMA`.
2. Chama `parse_lista_decl_globais(p)`.
3. Se a lista existir, adiciona-a como filho da raiz.

A raiz da AST e, portanto, sempre um programa com uma unica subarvore de declaracoes globais.

### `parse_lista_decl_globais`

Esta regra percorre os tokens ate `TOK_EOF`.

Para cada posicao:

1. Tenta `parse_declaracao_global(p)`.
2. Se conseguir, adiciona o resultado como filho da lista.
3. Se nao conseguir, consome um token para evitar ciclo infinito.

O no criado para esta lista e `NODE_LISTA_DECL_GLOBAIS`.

## 3. Declaracoes globais

### 3.1 Diretiva de preprocessador

`parse_declaracao_global` reconhece duas formas de diretiva:

- Forma tokenizada: `TOK_HASH` seguido de `TOK_INCLUDE`.
- Forma antiga/legada: `PREPROCESSOR_DIRECTIVE` como um unico token.

Quando encontra `#include`, distingue:

- `#include <ficheiro>` -> cria `NODE_DIRETIVA_INCLUDE` com filho `NODE_NOME_FICHEIRO` e marca `op = "system"`.
- `#include "ficheiro"` -> cria `NODE_DIRETIVA_INCLUDE` com filho `NODE_NOME_FICHEIRO` e marca `op = "local"`.

O nome do ficheiro fica em `valor` do no `NODE_NOME_FICHEIRO`.

### 3.2 typedef

Quando o token corrente e `KEYWORD_TYPEDEF`:

1. O parser consome `typedef`.
2. Le o tipo com `parse_especificador_tipo`.
3. Consome asteriscos opcionais com `parse_asteriscos`.
4. Lê o identificador do alias.
5. Remove sufixos de array opcionais com `parse_sufixo_array_opcional`.
6. Consome `;`.
7. Cria `NODE_DECLARACAO_TYPEDEF`.

O no typedef guarda o nome do alias em `valor` e o tipo original como filho.

### 3.3 Declaracao de funcao

Se apos o tipo e o identificador aparecer `(`, o parser interpreta como funcao.

Fluxo:

1. Lê o tipo base com `parse_especificador_tipo`.
2. Consome ponteiros opcionais com `parse_asteriscos`.
3. Lê o nome da funcao.
4. Consome `(`.
5. Processa os parametros com `parse_parametros_opcionais`.
6. Consome `)`.
7. Cria `NODE_DECLARACAO_FUNCAO`.
8. Adiciona o tipo como filho.
9. Se vier `;`, trata como prototipo.
10. Se vier `{`, chama `parse_bloco` e adiciona o corpo como filho.

No caso de prototipo, o parser cria um filho extra `NODE_IDENTIFICADOR` com valor `proto` para marcar que nao ha corpo.

### 3.4 Declaracao de variavel global

Se nao for funcao, o parser trata o resto como declaracao de variavel global.

Fluxo principal:

1. Lê o tipo.
2. Consome ponteiros opcionais.
3. Lê o primeiro identificador.
4. Processa sufixos de array opcionais.
5. Se houver `=`, analisa a expressao inicializadora.
6. Cria `NODE_DECLARACAO_VARIAVEL` para cada declarador.
7. Agrupa tudo num `NODE_LISTA_DECL_GLOBAIS`.
8. Repete o processo para declaradores separados por `,`.
9. Consome `;` no fim.

Cada variavel guarda o nome em `valor` e o tipo como filho. Quando existe inicializacao, a expressao tambem pode ser ligada como filho, dependendo do ponto de construcao.

## 4. Tipos, ponteiros e arrays

### `parse_especificador_tipo`

Reconhece:

- `KEYWORD_INT`
- `KEYWORD_FLOAT`
- `KEYWORD_CHAR`
- `KEYWORD_VOID`

Cria `NODE_ESPECIFICADOR_TIPO` com o texto do tipo em `valor`.

### `parse_asteriscos`

Consome zero ou mais `*` e devolve a quantidade lida.

O valor retornado e usado como parte da sintaxe de ponteiros, embora a AST nao crie um no proprio para cada asterisco nesta implementacao.

### `parse_sufixo_array_opcional`

Consome um ou mais sufixos do tipo `[` expressao `]`.

Na pratica, a funcao avanca o parser sobre a dimensao do array, mas nao cria um no dedicado para o acesso/declaração de array nesta versao.

### `parse_parametros_opcionais`

Usado na declaracao de funcoes.

- Se encontrar `KEYWORD_VOID`, consome o token e cria um `NODE_LISTA_DECL_GLOBAIS` com um filho `NODE_ESPECIFICADOR_TIPO` de valor `void`.
- Caso contrario, chama a logica de lista de parametros.

Os parametros individuais sao convertidos em `NODE_DECLARACAO_VARIAVEL` com o tipo como filho.

## 5. Corpo de funcoes e instrucoes

### `parse_bloco`

Um bloco comeca em `{`.

Fluxo:

1. Consome `{`.
2. Cria `NODE_BLOCO`.
3. Chama `parse_lista_itens_bloco`.
4. Adiciona a lista de itens como filho.
5. Exige `}` com `parser_expect`.

### `parse_lista_itens_bloco`

Percorre ate encontrar `}` ou `TOK_EOF`.

Para cada item:

1. Chama `parse_item_bloco`.
2. Se houver sucesso, adiciona o item ao no da lista.
3. Se falhar, ativa recuperacao de erro com `parser_sincronizar_geral`.

O no usado aqui tambem e `NODE_LISTA_DECL_GLOBAIS`, funcionando como lista generica de itens.

### `parse_item_bloco`

Tenta primeiro uma declaracao local e, se nao for isso, tenta uma instrucao.

Ordem:

1. Se o token atual parecer inicio de tipo ou identificador, chama `parse_declaracao_variavel_local`.
2. Caso contrario, chama `parse_instrucao`.

### `parse_declaracao_variavel_local`

Reconhece declaracoes locais com o mesmo padrao base das globais.

Fluxo:

1. Lê o tipo.
2. Consome asteriscos opcionais.
3. Lê o identificador.
4. Processa arrays opcionais.
5. Processa inicializacao se existir `=`.
6. Processa declaradores adicionais separados por `,`.
7. Exige `;` com `parser_expect`.
8. Cria `NODE_DECLARACAO_VARIAVEL`.

### `parse_instrucao`

Seleciona a regra pela forma do token corrente:

- `{` -> `parse_bloco`
- `KEYWORD_IF` -> `parse_instrucao_if`
- `KEYWORD_WHILE` -> `parse_instrucao_while`
- `KEYWORD_FOR` -> `parse_instrucao_for`
- `KEYWORD_DO` -> `parse_instrucao_do`
- `KEYWORD_RETURN` -> `parse_instrucao_return`
- `;` -> instrucao vazia, devolvendo `NODE_INSTR_EXPR`
- caso contrario -> `parse_instrucao_expressao`

### `parse_instrucao_expressao`

Analisa uma expressao seguida de `;`.

Se a expressao falhar, emite erro e sincroniza.
Se faltar `;`, emite erro e pode sincronizar, salvo quando a palavra seguinte permitir que a regra superior continue o processamento.

O resultado final e um `NODE_INSTR_EXPR` com a expressao como filho.

### `parse_instrucao_if`

Estrutura:

1. Consome `if`.
2. Exige `(`.
3. Lê a condicao com `parse_expressao`.
4. Exige `)`.
5. Lê a instrucao corpo com `parse_instrucao`.
6. Cria `NODE_IF` com condicao e corpo como filhos.

### `parse_instrucao_while`

Estrutura equivalente ao `if`, mas cria `NODE_WHILE`.

### `parse_instrucao_for`

Fluxo:

1. Consome `for`.
2. Exige `(`.
3. Lê a inicializacao, se nao encontrar `;` imediatamente.
4. Exige o primeiro `;`.
5. Lê a condicao, se nao encontrar `;` imediatamente.
6. Exige o segundo `;`.
7. Lê a expressao de incremento, se nao encontrar `)` imediatamente.
8. Exige `)`.
9. Lê o corpo com `parse_instrucao`.
10. Cria `NODE_FOR` com init, condicao, incremento e corpo.

### `parse_instrucao_do`

Fluxo:

1. Consome `do`.
2. Lê o corpo com `parse_instrucao`.
3. Exige `while`.
4. Exige `(`.
5. Lê a condicao.
6. Exige `)`.
7. Exige `;`.
8. Cria `NODE_DO_WHILE`.

### `parse_instrucao_return`

Fluxo:

1. Consome `return`.
2. Se nao houver `;` logo a seguir, analisa uma expressao.
3. Exige `;`.
4. Cria `NODE_RETURN` com a expressao, se existir, como filho.

## 6. Expressoes e precedencia

O parser de expressoes segue descida recursiva e associa os operadores por precedencia crescente.

### Ordem real de chamada

`parse_expressao` -> `parse_atribuicao` -> `parse_logico_ou` -> `parse_logico_e` -> `parse_bit_ou` -> `parse_bit_xor` -> `parse_bit_e` -> `parse_igualdade` -> `parse_bit_shift` -> `parse_relacional` -> `parse_aditivo` -> `parse_multiplicativo` -> `parse_unario` -> `parse_pos_fixo` -> `parse_primario`

### `parse_primario`

Reconhece:

- `INT_LITERAL`
- `FLOAT_LITERAL`
- `IDENTIFIER`
- `STRING_LITERAL`
- `CHAR_LITERAL`
- expressoes entre parenteses

Neste ponto, os literais e identificadores sao representados com `NODE_IDENTIFICADOR` nesta implementacao, usando o texto do token em `valor`.

### `parse_pos_fixo`

Aplica operadores pos-fixos sobre o primario:

- acesso a array: `expr[ idx ]` -> `NODE_ACESSO_ARRAY`
- chamada de funcao: `expr( args )` -> `NODE_CHAMADA_FUNC`
- acesso a membro: `expr.ident` -> `NODE_ACESSO_MEMBRO`
- incremento/decremento pos-fixo: `expr++` / `expr--` -> `NODE_POS_FIXO`

### `parse_unario`

Reconhece operadores unarios:

- `!`
- `-`
- `&`
- `*`

Cada um cria `NODE_OP_UNARIO` com o operador em `op` e a subexpressao como filho.

### Operadores binarios

Cada nivel constrói um `NODE_OP_BINARIO` e liga:

- filho 0: lado esquerdo
- filho 1: lado direito

Os niveis sao:

- multiplicativo: `*`, `/`, `%`
- aditivo: `+`, `-`
- relacional: `<`, `>`, `<=`, `>=`
- shift: `<<`, `>>`
- igualdade: `==`, `!=`
- bit a bit e: `&`
- bit a bit xor: `^`
- bit a bit ou: `|`
- logico e: `&&`
- logico ou: `||`

### `parse_atribuicao`

Reconhece atribuicao associativa a direita:

- `=`
- `+=`
- `-=`
- `*=`
- `/=`
- `%=`

Quando encontra um destes operadores, cria `NODE_ATRIBUICAO` com:

1. filho esquerdo: a expressao alvo
2. filho direito: a expressao do lado direito

O operador fica em `op`.

### `parse_argumentos_opcionais`

Usado em chamadas de funcao.

Cria um `NODE_LISTA_DECL_GLOBAIS` com cada argumento como filho, separados por virgulas. Se o proximo token ja for `)`, devolve `NULL`.

## 7. Como a AST e montada

### Estrutura base dos nos

Os nos sao criados em `ast/ast.c`:

- `make_node(type, linha)` cria o no com filhos vazios.
- `make_folha(type, valor, linha)` cria um no folha com texto em `valor`.
- `add_filho(pai, filho)` anexa o filho no fim da lista de filhos.

Cada no tem:

- `type` - tipo semantico do no.
- `valor` - texto associado, como nome de identificador ou alias.
- `op` - operador guardado em operadores binarios, unarios ou pos-fixos.
- `linha` - linha de origem do token principal.
- `filhos` - vetor dinamico de filhos.

### Padrao de construcao

O parser segue um padrao simples:

1. Consome tokens da entrada.
2. Decide que regra aplica.
3. Cria o no AST da construcao atual.
4. Anexa os elementos internos como filhos na ordem em que aparecem.

### Exemplos de formato na AST

- Programa: `NODE_PROGRAMA` -> lista de declaracoes globais.
- Funcao: `NODE_DECLARACAO_FUNCAO` -> tipo + parametros/prototipo/corpo.
- Variavel: `NODE_DECLARACAO_VARIAVEL` -> tipo + inicializacao opcional.
- Bloco: `NODE_BLOCO` -> lista de itens.
- If/While/For/Do: no da instrucao -> subnos de condicao, corpo e partes auxiliares.
- Expressao binaria: `NODE_OP_BINARIO` -> esquerda e direita.

### Impressao

`print_ast` percorre a arvore em profundidade e imprime:

- o tipo do no,
- a linha,
- `valor`, quando existe,
- `op`, quando existe.

Isto e util para validar se a estrutura criada pelo parser corresponde ao codigo de entrada.

## 8. Recuperacao de erro

O parser usa um modo de sincronizacao simples em `parser_sincronizar_geral`.

Comportamento:

1. Regista a linha do erro.
2. Avanca tokens ate encontrar `;`, `}` ou `TOK_EOF`.
3. Se a linha mudar, para para evitar consumir demasiado.

`parser_expect` usa esse mecanismo quando um token esperado nao aparece.

Em resumo, a estrategia e de pânico controlado: tenta recuperar no fim da instrucao ou no fecho de bloco.

## 9. Resumo curto do fluxo real

1. O programa e tokenizado.
2. A raiz `NODE_PROGRAMA` e criada.
3. O parser percorre declaracoes globais ate EOF.
4. Cada declaracao global pode ser diretiva, typedef, funcao ou variavel.
5. Dentro de funcoes, blocos chamam lista de itens, que alterna entre declaracao local e instrucao.
6. Expressoes sao construidas por precedencia, com associatividade correta para atribuicao.
7. Cada regra cria nos especificos e liga os filhos na ordem sintatica original.

Se quiseres, este documento pode ser expandido depois com exemplos concretos de entrada e a respetiva AST impressa.