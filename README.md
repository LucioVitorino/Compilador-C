# Compilador-C

## Descrição
Este projecto é um analisador léxico simples para a linguagem C. O programa lê um ficheiro fonte em C e produz uma lista de tokens (identificadores, keywords, literais, operadores, comentários, delimitadores, etc.). O código está organizado em pastas: `Lexer/` (lógica do lexer), `utils/` (funções utilitárias) e ficheiros de teste no root.

A crianção do lexer teve como base fundamental os conceitos de Linguagens Formais e Autómatos, como autômatos finitos, conceitos de máquinas de estado, expressões regulares só para mencionar alguns.

O objectivo é fornecer um lexer educacional que reconheça operadores, identificadores, literais e comentários, e que sirva de base para um compilador mais completo.

## Execução
Passos para compilar e executar o projecto no Linux (bash):

1. Abrir um terminal no directório do projecto (onde está o `Makefile`).

2. Compilar: make

Isto gera o executável `analisador_lexico` na raiz do projecto.

3. Executar o analisador sobre um ficheiro de teste (por exemplo `test_operators.c`):

```bash
./analisador_lexico test_operators.c
```

A saída imprime uma lista de tokens no formato: `value= <valor>, type= <tipo>, line= <linha>`.

Notas úteis:
- Se alterar ficheiros `.c` ou `.h`, executar `make` de novo. Para limpar objectos use `make clean` e para remover executável `make fclean`.
- O `Makefile` inclui flags de debug (`-g`) que ajudam a usar ferramentas como `gdb` ou `valgrind`.

## Recursos (Tecnologias e Metodologia)
- Linguagem: C (compatível com gcc)
- Ferramentas de construção: Make (`Makefile` no root)
- Utilitários usados durante desenvolvimento: `gdb`, `valgrind` (para debugging/diagnóstico de segfaults e leaks)

Metodologia seguida:
- Implementação incremental do lexer — funções pequenas e responsáveis por apenas um tipo de token (identificadores, strings/chars, operadores, comentários, literais numéricos).
- Uso de testes manuais com ficheiros como `test_operators.c` para validar reconhecimento de operadores e comentários.
- Debug com `gdb` e `valgrind` quando ocorreram erros de memória (segfaults ou leitura inválida).

## Uso de IA
Durante o desenvolvimento e diagnóstico deste projecto, a IA foi utilizada para:
- Analisar o código e identificar possíveis acessos fora dos limites e chamadas com ponteiros errados (por exemplo, uso de `line['<']` em vez de `line[i]`).
- Propor correções de forma segura (pequenas e localizadas) para evitar segfaults: correção de condições lógicas, passagem correta de ponteiros e inicialização de variáveis.
- Sugerir melhorias de robustez, como validações defensivas em funções que manipulam strings e comentários multilinha.

A IA foi utilizada como assistente de programação (pair-programming): sugeriu patches, executou compilação e testes locais, e produziu recomendações explicando as causas dos problemas e como confirmar as correções com `gdb`/`valgrind`.

## Testes e verificação rápida
- Teste rápido com `test_operators.c` incluído no repositório:

```bash
make
./analisador_lexico test_operators.c
```

- Para análise de memória/segfaults:

```bash
valgrind --leak-check=full --track-origins=yes ./analisador_lexico test_operators.c
```

