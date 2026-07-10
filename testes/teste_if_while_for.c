#include <stdio.h>

int main() {
    int x = 5;       // Erro 1: Falta o valor/expressão na inicialização
    int y = 10;     // Deve ser recuperado com sucesso

    if (x == 10) {
        y = 20;     // Deve ser mapeado como o corpo do IF após a recuperação

    }  // Erro 2: Falta o parêntese de fecho ')'

    while (x < 20) { // Erro 3: Falta o parêntese de abertura '('
        x++;
    }

    int i = 0;
    int soma = 0;

    // 1. Teste do FOR Correto (Cenário Ideal)
    for (i = 0; i < 10; i = i + 1) {
        soma = soma + i;
    }

    // 2. Teste de Modo Pânico no FOR (Erro Propositado: falta o ';' após a condição)
    for (i = 0; i < 5  i = i + 1) {
        soma = soma + 1;
    }

    // 3. Instrução seguinte para garantir que o parser recuperou do pânico com sucesso
    int controlo = 100;
    return 0;

    return 0;
}