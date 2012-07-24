#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct element {
	struct element *next, *prev;
	int count;
	char *string;
};

void merge_sort (char x[][20], int n)
{
	char aux[10][20];
	int i, j, k, l1, l2, size, u1, u2;
	void *point = (void *)malloc(sizeof(void *));
printf("%p\n", point);
	size = 1;
	while(size < n) {
		l1 = 0; /* first lower bound */
		k = 0; /* aux array index */
		while(l1 + size < n) {
			l2 = l1 + size; /* second lower bound */
			u1 = l2 - 1;	/* first upper bound */
			u2 = (l2 + size - 1 < n) ? l2 + size - 1: n - 1;
			for(i = l1, j = l2; i <= u1 && j <= u2 ; k++) 
				if(strcmp(x[i], x[j]) < 0)
					strcpy(aux[k], x[i++]);
				else
					strcpy(aux[k], x[j++]);
			/* all is saturated, need to copy after this.
			 * insert remaining array at the end of other files
			 */
			for(; j <= u2; k++) 
				strcpy(aux[k], x[j++]);
			for(; i <= u1; k++) 
				strcpy(aux[k], x[i++]);

			l1 = u2 + 1;
		}

		for(i = l1; k < n; i++) 
			strcpy(aux[k++], x[i]);
		for(i = 0; i < n; i++)
			strcpy(x[i], aux[i]);
		size *= 2;
	}
		for(i = 0; i < n; i++) {
			printf(" %s ", x[i]);
		}
}

int main(void) 
{
	struct element list;
	char array[][20] = {"one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten"};
	int num = sizeof(array)/sizeof(array[0]);
	printf("%d\n",num);
	merge_sort(array, num);
	return 0;
}
