#include <stdio.h>
#include<string.h> 
char base64(char c);
void encode(unsigned char* s,unsigned char* f);

int main()
{
unsigned char s[12];
unsigned char f[4];
int i,tail;
fgets(s, 12, stdin);

tail = strlen(s)%3;
for(i=0; i<strlen(s)-tail; i+=3){
	encode(s+i, f);
	printf("%c%c%c%c\n", base64(f[0]),base64(f[1]),base64(f[2]),base64(f[3]));
	}
if(tail == 1) {
	s[i+1] = 0; //e' gia' il terminatore
	encode(s+i,f) ;
	printf("%c%c==\n", base64(f[0]),base64(f[1]));
}
else if(tail == 2) {
	s[i+2] = 0; //e' gia' il terminatore
	encode(s+i,f) ;
	printf("%c%c%c=\n", base64(f[0]),base64(f[1]),base64(f[2]));
	}
}
void encode(unsigned char* s,unsigned char* f){
f[0] = s[0] >> 2;
f[1] = ((unsigned char)(s[0] << 6) | (s[1] >> 2)) >> 2;
f[2] = ((0x0F & s[1]) << 2) |(s[2] >> 6);
f[3] = s[2] & 0x3F;
}

char base64(char c){
switch(c){
	case 0 ... 25: return ('A' + c);
	case 26 ... 51: return ('a' + c-26);
	case 52 ... 61: return (c-52+ '0');
	case 62: return '+';
	case 63: return '/';
}
}




