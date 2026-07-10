int soma(int a, float b) {
    return 0;
}

typedef struct Ponto {
    int x;
    int y;
} Ponto;

int main() {
    // 1. Variável não declarada
    x = 10;
    
    // 2. Redeclaração no mesmo escopo
    int redecl;
    float redecl;
    
    // 3. Incompatibilidade de tipos na atribuição
    int b;
    b = "ola";
    
    // 4. Erros em argumentos de funções
    soma(1);
    soma(1, "erro");
    
    // 5. Erros em condições de controle
    if ("incompativel") {
        b = 1;
    }
    
    // 6. Membro inexistente na struct
    Ponto p;
    p.z = 100;
    
    // 7. Retorno de tipo incompatível (função espera int, retorna float/void/etc se for incompatível)
    // Nota: float é compatível com int em C padrão mas "string" não é
    return "incompativel";
}
