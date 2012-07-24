#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

struct element {
	struct element *next, *prev;
	int count;
	char *string;
};

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

struct element * populate_the_list(void)
{
	struct element *temp, * head, *new;
	int i = 0;
	char string[40];
	head = NULL;
	struct timeval tv;
	while(i < 20) {
		int status;
		temp = (struct element *)malloc(sizeof(struct element));
		status = gettimeofday(&tv, NULL);
		printf("%p %d %ld %d \n", &tv, status, tv.tv_usec, rand());
		memset(string, '0', sizeof(string));
		sprintf(string, "%d%ld", rand(), tv.tv_usec);
		temp->count = i;
		temp->string = (char *)malloc(strlen(string)+1);
		strcpy(temp->string, string);
		temp->next = NULL;

		if(!head) {
			new = temp;
			head = new;
		} else {
			new->next = temp;
			new = new->next;	
		}
		temp = temp->next;
		i++;
	}

	return head;
}

void free_list(struct element *list)
{
	struct element *temp;
	while(list){
		temp = list->next;	
		free(list->string);
		list->string = NULL;
		free(list);
		list = NULL;
		list = temp;
	}
}

int main(void) 
{	
	struct element *list, *temp;
 	temp = populate_the_list();
	list = temp;
	temp = sort_list(list);
	while(temp) {
		printf("%s\n", temp->string);
		printf("%d\n", temp->count);
		temp = temp->next;	
	}
	free_list(list);
	return 0;
}
