#include <stdio.h>

int main (void)
{

struct struct1 {
int a;
char b;
float c;
double d;
short int e;
long int f;
};

struct struct1 s;

printf("start of int %p\n", &s.a);
printf("%ld\n", sizeof(s.a));
printf("start of char %p\n", &s.b);
printf("%ld\n", sizeof(s.b));
printf("start of float %p\n", &s.c);
printf("%ld\n", sizeof(s.c));
printf("start if double %p\n", &s.d);
printf("%ld\n", sizeof(s.d));
printf("start of short int %p\n", &s.e);
printf("%ld\n", sizeof(s.e));
printf("start of long int %p\n", &s.f);
printf("%ld\n", sizeof(s.f));
printf("%ld\n", sizeof s);
	return 0;
}
