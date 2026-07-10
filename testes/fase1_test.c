#define MAX_VAL 100
#define MESSAGE "Ola Mundo"

int main() {
    int x = 42;
    float f = 3.14;
    char c = 'A';
    
    do {
        x = x + 1;
    } while (x < 50);
    
    for (; x < MAX_VAL; ) {
        x = x + 2;
    }
    
    return 0;
}
