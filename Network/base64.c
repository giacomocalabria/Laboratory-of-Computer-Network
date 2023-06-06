#include <stdio.h>
#include <stdlib.h>

static const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int main(){
    unsigned char *str = "ab"; // 011000|01 0110|0010
    printf("%x\n", str[0]);
    printf("%x\n", str[1]);

    unsigned char *c = malloc(3);
    c[0] = str[0] >> 2; // 011000
    c[1] = (unsigned char) str[0] << 6;// | (unsigned char) str[1] >> 4; // 010110
    c[2] = (unsigned char) str[1] << 4; // 001000

    printf("\n");

    printf("%x\n", c[0]);
    printf("%x\n", c[1]);
    printf("%x\n", c[2]);

    return 0;
}

char base64(char c){
    switch(c){
        case 0 ... 25:
            return c + 'A';
        case 26 ... 51:
            return c + 'a' - 26;
        case 52 ... 61:
            return c + '0' - 52;
        case 62:
            return '+';
        case 63:
            return '/';
    }
}
