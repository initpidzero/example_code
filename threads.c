#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

void *thread_function(void *arg) {
	int i;
	int *value = (int *)malloc(sizeof(int));
	
	*value = (*(int *)arg);
	printf("vlaue of arg %d\n", *value);
	for ( i=0; i<20; i++ ) {
		printf("Thread says hi! %d \n", (*value)++);
		sleep(1);
	}
	return value;
}

int main(void) {

	pthread_t mythread;
	int arg = 10;
	void *ret;
	if ( pthread_create( &mythread, NULL, thread_function, &arg) ) {
		printf("error creating thread.");
		abort();
	}

	printf("error joining thread.");
	if ( pthread_join ( mythread, &ret) ) {
		printf("error joining thread.");
		abort();
	}

	printf("value of arg %d\n",*(int *)ret);
	printf("value of arg %p\n",(int *)ret);
	printf("value of arg %p\n",ret);
	free(ret);
	exit(0);

}
