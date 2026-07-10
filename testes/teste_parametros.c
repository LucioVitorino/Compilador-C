int main() {
    int x = 10;
    int y = 20;

    // 1. Chamada Correta
    printf("Soma: %d", x + y);

    // 2. Erro Propositado: Esqueceu o parêntese de fecho
    printf("Valores: %d %d", x, y ; 

    // 3. Linha de controlo local para verificar integridade do bloco
    int controlo = 777;

    // 4. Erro Propositado: Vírgula órfã no fim da chamada
    printf("Erro aqui", x, );

    return 0;
}