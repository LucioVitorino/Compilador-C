int main() {
    int x = 0;

    // 1. do/while Correto
    do {
        x++;
    } while (x < 10);

    // 2. Erro Propositado: Falta o ';' no final
    do {
        x--;
    } while (x > 0) // <-- Sem ponto e vírgula

    int controlo = 200;
    return 0;
}