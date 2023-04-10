#include <stdio.h>

struct tipo {
	char x;
	long  y; 
};

int main()
{
char *p;
struct tipo t;

p = (char *) &t.y;
printf("t.y e' all'indirizzo %lx\n", p);
p = &t.x;
printf("t.x e' all'indirizzo %lx\n", p);

}

