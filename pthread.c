#include <stdio.h> 
#include <unistd.h> 
#include <stdlib.h> 
#include <string.h> 
#include <pthread.h>

char message[] = "Hello World";
int run_now = 1;

void *thread_function(void *arg) {
	int print_count2 = 0;
	//printf("thread_function is running. Argument was %s\n", (char *)arg); sleep(3);
	//strcpy(message, "Bye!");
	//pthread_exit("Thank you for the CPU time");
	while(print_count2++ < 20) { 
		if (run_now == 2) {
			printf("2");
			run_now = 1; 
		} else { 
			sleep(1);
		} 
	}
}


int main(void) 
{ 
	int res;
	pthread_t a_thread; 
	void *thread_result;
	int print_count1 = 0;
	res = pthread_create(&a_thread, NULL, thread_function, (void *)message); 

	while(print_count1++ < 20) { 
		if (run_now == 1) {
			printf("1");
			run_now = 2; 
		} else { 
			sleep(1);
		} 
	}

	if (res != 0) {
		perror("Thread creation failed");
		exit(EXIT_FAILURE); 
	}

//	printf("Waiting for thread to finish...\n"); 
	res = pthread_join(a_thread, &thread_result); 
	if (res != 0) {
		perror("Thread join failed");
		exit(EXIT_FAILURE); 
	}

       printf("Thread joined, it returned %s\n", (char *)thread_result); 
      // printf("Message is now %s\n", message);

       exit(EXIT_SUCCESS);
}
