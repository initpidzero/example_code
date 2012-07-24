#include <stdio.h>
#include <stdlib.h>
int main(int argc, char* argv[])
{
	unsigned long int num = 0xFF00100190;
	unsigned int i = sizeof(num)*8;
	unsigned char *string;
	unsigned long int shift = 1;

	if(argc == 2)
		sscanf(argv[1],"%x",&num);
	
       	string = (unsigned char *)malloc(i);	
	printf("%d\n",sizeof(string));
	string[i] = '\0';
	for(;shift != 0; shift <<= 1) {
		if(num &  shift ) string[--i] = '1';
		else string[--i] = '0';
	}

	printf("%lx\n",num);
	puts(string);

	free(string);
	string = NULL;
	return 0;
}
