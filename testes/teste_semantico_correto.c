typedef struct Ponto {
    int x;
    int y;
} Ponto;

int soma(int a, float b) {
    return a;
}

int main() {
    Ponto p;
    p.x = 10;
    p.y = 20;
    
    int val = soma(p.x, 3.14);
    
    if (val < 50) {
        val = val + 1;
    }
    
    return val;
}
