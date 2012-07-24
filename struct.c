#include <stdio.h>

int main (void)
{

	struct mystruct {
	char *whatever;
	int new;

	} newstruct[2];

	printf("%d\n", &newstruct[1] - &newstruct[0] );
	printf("%d\n", (void *)&newstruct[1] - (void *)&newstruct[0] );
	printf("%d\n", (char *)&newstruct[1] - (char *)&newstruct[0] );
	printf("%d\n", (struct mystruct *)&newstruct[1] - (struct mystruct *)&newstruct[0] );

	return 0;
}
