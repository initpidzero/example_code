#include <stdio.h> 
#include <unistd.h> 
#include <stdlib.h> 
#include <string.h> 
#include <pthread.h> 
#include <semaphore.h>
#include <errno.h>

extern int errno;
sem_t bin_sem;
#define WORK_SIZE 1024 
char work_area[WORK_SIZE];
void error_exit(char *str)
{

	fprintf(stderr,"%s", str);
	exit(EXIT_FAILURE);
}

void * thread_function(void * args)
{
 	sem_wait(&bin_sem);
	while(strncmp("end",work_area,3) != 0) {
		printf("input characters %d \n", (int)strlen(work_area) - 1);
		sem_wait(&bin_sem);
	}
	pthread_exit(NULL);
}

int main(void) 
{
	int res;
	pthread_t a_thread;
	void *thread_result;

#ifdef  _POSIX_SEMAPHORES
	printf("defined\n");
#else
	printf("no\n");
#endif

		
	res = sem_init(&bin_sem, 0, 0);
	if(res != 0) {
		printf("%s\n",strerror(errno));
		error_exit("semaphore init error \n");
	}

	res = pthread_create(&a_thread, NULL, thread_function, NULL);
	if(res != 0) {
		error_exit("pthread create error");
	}


	printf("text for consumption and end for stopping\n");
	while(strncmp("end", work_area,3) != 0) {
		fgets(work_area, WORK_SIZE, stdin);
		/*  lock is to basically increment the value */
		sem_post(&bin_sem);
	
	}
	printf("Wait for thread to finish....\n");
	res = pthread_join(a_thread, &thread_result);
	if(res != 0) {
		error_exit("join error");
		
	}
	printf("Thread join\n");
	sem_destroy(&bin_sem);
	return 0;
}
