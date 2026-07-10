
#include <>
#include "Lexer/token.h"

int main()
{
    int x = 5;
    int y = 10;
    float f = 9.8;
    
    x++;
    y--;
    
    x = x + 1;
    y = y - 2;
    if (x == y || x >= 1 && x <= y)
    x /= 2;
    x *= 1;
    x += 1;
    x -= 2; 

    return 0;
}


int soma(int a, int b) {
    int soma = a + b;
    return soma;
}