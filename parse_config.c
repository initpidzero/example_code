#include <stdio.h>
#include <string.h>
#include "pthread_common.h"

const char compare_string[][256] = {"pthread_count","child_count"};

void merge_sort (char x[][BUFSIZ], int n)
{
	char aux[8][BUFSIZ];
	int i, j, k, l1, l2, size, u1, u2;

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

int parse_file(struct parameter * param)
{
	int res,i = 0;
	char *tok, buf[256];
	char delim [] = ":";
	FILE *fp = fopen(param->filepath, "r");

	if(!fp) {
		anuz_debug( "fopen failed \n");	
	}

	while ((tok = fgets(buf, sizeof(buf), fp)) != NULL) {
		anuz_debug("buffer = %s", buf);
		 tok = strtok(buf, delim);		
		anuz_debug("tok = %s\n",tok);
		 if(!strcmp(compare_string[i], tok)) {
			 tok = strtok(NULL, delim);		
			 anuz_debug("tok = %s",tok);
			 strcpy(param->params[i++], tok);
		 }
	}
	fclose(fp);
}

void free_list(struct element *list)
{
	struct element *temp;
	while(list){
		temp = list->next;	
		anuz_debug("inside free = %d\n",list->count);
		free(list->string);
		list->string = NULL;
		free(list);
		list = NULL;
		list = temp;
	}
}

struct element * sort_list(struct element *list)
{
	struct element *p, *q, *tail, *e;
	int insize, nmerges, psize, qsize, i;

	if(!list)
		return NULL; /* why would you send a empty list */

	insize = 1;
	
	while(1) {
		p = list;
		list = NULL;
		tail = NULL;

		nmerges = 0;

		while(p) {
			nmerges++;
			q = p;
			psize = 0;
			/* this entire loop is to get position for q */
			for(i = 0; i < insize; i++ ) {
				psize++;
				q = q->next;
				if(!q) break;
			}
			/* both p and q should be at same size */
			qsize = insize;
			while ( psize > 0 || (qsize > 0 && q) ) {
				if(psize == 0) {
					/* from q */
					e = q;
					q = q->next;
					qsize--;
				} else if( qsize == 0 || !q){
					/* from p */
					e = p; 
					p = p->next;
					psize--;
				} else if (strcmp(p->string,q->string) <=0 ) {
					e = p; 
					p = p->next; 
					psize--;
				} else {
					e = q;
					q = q->next;
					qsize--;
				}
			/* this is the actual list */	
				if(tail) {
					tail->next = e;
				} else {
					list = e;
				}
				tail = e;
			}
			/* p and q have moved3 along 'insize' times */
			p = q;
		}

		tail->next = NULL;

		if(nmerges <= 1) /* only one merge required, we are pretty much done */
			return list;

		insize *= 2; /* repeat with double size */
	}
}
