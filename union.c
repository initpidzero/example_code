#include <stdio.h>
#include <stdlib.h>
void represent(char *argv)
{
	
	unsigned long int num = 0xFF001001;
	unsigned int i = sizeof(num)*8;
	unsigned char *string;
	unsigned long int shift = 1;

	sscanf(argv,"%lx",&num);
	printf("number = %lx\n",num);
	
       	string = (unsigned char *)malloc(i);	
	string[i] = '\0';
	for(;shift != 0; shift <<= 1) {
		if(num &  shift ) string[--i] = '1';
		else string[--i] = '0';
	}

	puts(string);

	free(string);
	string = NULL;
}

int main (void) 
{
	void *mem, *pointer;
	union {
		unsigned short int x; 
		unsigned char y[2];
		unsigned char z;
		unsigned char w;
	} u1;
	char string[BUFSIZ];
	struct str1 {
		int x;
	} str[2];
	u1.x = 0xFFAA;
	printf("struct =%ld %ld\n",sizeof(struct str1),(size_t)&str[1]-(size_t)&str[0]);
	mem = malloc(1024+15);
	pointer = (void*)(((size_t)mem+15) & ~(size_t)0x0f);

	printf("mem  = %p\n",mem);
	printf("pointer = %p\n",pointer);

	sprintf(string, "%lx",pointer);
	puts(string);
	represent(string);
	sprintf(string, "%lx",mem);
	puts(string);
	represent(string);

	printf("%p  %p %p %p %p\n",&u1.x, &u1.y[0], &u1.y[1], &u1.z, &u1.w);
	printf("%hx %hx %hx %hx %hx\n",u1.x, u1.y[0], u1.y[1], u1.z, u1.w);
	u1.y[0] = 0xBB;
	u1.y[1] = 0xAA;
	printf("%hx %hx %hx %hx %hx\n",u1.x, u1.y[0], u1.y[1], u1.z, u1.w);
	printf("%hx %hx %hx %hx %hx\n",*(char*)&u1.x, *(char*)(&u1.x+1), u1.y[1], u1.z, u1.w);
	printf("%d\n",sizeof(u1));
	printf("%d\n",0x0F);
	printf("%d\n",0xF);
	printf("%d\n",0xF0);

	return 0;

}
