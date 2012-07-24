#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct new {
	int i;
};

void fun( struct new **new)

{
	printf("%x\n",*new);
	printf("%x\n",new);
	if(*new == NULL)
		printf("what\n");
	else
		printf("not null\n");
	*new = (struct new *)malloc(10);
	printf("%x\n",new);

	printf("%d", **new);
}

int main()
{
	struct new *new ;      
	fun(&new);
	//      printf("%d\n",__LINE__);

	return 0;
}

