#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <semaphore.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/time.h>
#include <pthread.h>

#define MAX_PORT 4

static pthread_mutex_t work_mutex[MAX_PORT]; /* the lock this file needs */

extern int errno;
char SEM_NAME[]= "vik";
struct  sock_s {
	struct sockaddr_in sin;
	unsigned short int port;
	int sd;
};


struct local_s {
	struct sockaddr_in pin;
	int sock;
	fd_set active;
	int i;
	int *count;
	char input[BUFSIZ];
	char input1[BUFSIZ];
	int fd_pipe;
	pthread_mutex_t lock;
};

short unsigned int ports  = 0x9123;


void populate_sock(int *sock, struct sockaddr_in *sin, const unsigned short int port)
{
	if ((*sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	memset(sin, 0, sizeof(*sin));
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = INADDR_ANY;
	sin->sin_port = htons(port);

	if (bind(*sock, (struct sockaddr *) sin, sizeof(*sin)) == -1) {
		perror("bind");
		close(*sock);
		exit(EXIT_FAILURE);
	}
}

void get_me_the_fds(fd_set active_sd, int *fd)
{
	int status;
	int i;
	fd_set read_sd; 
	FD_ZERO(&read_sd);
	read_sd = active_sd;	
	if ((status = select (FD_SETSIZE, &read_sd, NULL, NULL, NULL)) < 0) {
		printf("%s %d\n",strerror(errno), __LINE__);
		exit (EXIT_FAILURE);
	}
	printf("number of descriptor %d\n", status);
	for(i = 0; i < FD_SETSIZE; ++i)
		if (FD_ISSET (i, &active_sd)) {
			*fd = i; 
		}
}

void log_stuff(const char *message, int parameter)
{
	char msg[BUFSIZ];
	
	memset(msg, 0, sizeof(msg));
	sprintf(msg, "echo %s %d >> /tmp/error.txt", message, parameter);
	system(msg);

}

void get_me_the_buffer(int sock, char*  input, struct sockaddr_in pin)
{
	char buf[BUFSIZ];
	int i;
	int addrlen = sizeof(pin); 

	memset(buf, 0, sizeof(buf));
	if( recvfrom(sock , buf, sizeof(buf), 0, (struct sockaddr *)&pin, &addrlen) == -1 ) {
		printf("recvfrom error");
	}
	strcpy(input, buf);
	printf("%s\n",buf);
}

void child_stuff( struct local_s *local)
{
	memset(local->input, 0, sizeof(local->input));
	get_me_the_fds(local->active, &local->sock );
	get_me_the_buffer(local->sock, local->input, local->pin);
/*	memset(local->input1, 0, sizeof(local->input1));
	sprintf(local->input1, "child-%d,count-%d\n",local->i,*local->count);*/
	pthread_mutex_unlock(&local->lock);
	strcat(local->input, local->input1); 
	write(local->fd_pipe,local->input, sizeof(local->input));
	pthread_mutex_lock(&local->lock);

}

void error_exit(char *str)
{

	fprintf(stderr,"%s", str);
	exit(EXIT_FAILURE);
}

void * thread_function(void * args)
{
	struct local_s * local = (struct local_s *)args;
	int *count;
	
	pthread_mutex_lock(&local->lock);
	count = local->count;
	pthread_mutex_unlock(&local->lock);
	while(*count < 10) {
		pthread_mutex_lock(&local->lock);
		memset(local->input1, 0, sizeof(local->input1));
		sprintf(local->input1, "__In_Thread-child-%d,count-%d\n",local->i,*local->count);
		pthread_mutex_unlock(&local->lock);
		child_stuff(local);
		pthread_mutex_lock(&local->lock);
		(*count)++; /*if at all this works */
		pthread_mutex_unlock(&local->lock);
	}

	pthread_exit("Thank you for the CPU time");
}

int main(void) 
{
	int pd[4][2];
	int pid[4];
	char buf[BUFSIZ];
	int i = 0;
	int status;
	fd_set active;
	fd_set readf;
	int count = 0, count1 = 0;
	fd_set active_sd, read_sd; /* i am running short on names */
	struct sock_s s1[4]; /*cute names and stuff */
	int fd1;

	fd1 = open ("/tmp/whatever.txt", O_CREAT |  O_RDWR | O_TRUNC, 0644);
	printf("parent id %d\n",(int)getpid());

	FD_ZERO(&active);
	FD_ZERO(&readf);
	FD_ZERO(&active_sd);
	
	for(;i <4;i++){
		pipe(pd[i]);
		FD_SET(pd[i][0], &active);
		s1[i].port = ports + i; 
			 
		populate_sock(&s1[i].sd, &s1[i].sin, s1[i].port);
		FD_SET(s1[i].sd, &active_sd);

		pid[i] = fork();
		if(pid[i] == 0) {
			struct local_s local_data;
			int res;
			pthread_t a_thread;
			void *thread_result;

			res = pthread_mutex_init(&work_mutex[i], NULL);
			if(res != 0) {
				error_exit("mutex init error \n");
			}

			local_data.active = active_sd;
			local_data.i = i;
			local_data.fd_pipe = pd[i][1];
			local_data.lock = work_mutex[i];
			local_data.count = &count;

			res = pthread_create(&a_thread, NULL, thread_function, (void *)&local_data); 
			if(res != 0) {
				error_exit("thread creation error \n");
			} 
			
			while(count < 10) {
				pthread_mutex_lock(&local_data.lock);
				memset(local_data.input1, 0, sizeof(local_data.input1));
				sprintf(local_data.input1, "__In_Main-child-%d,count-%d\n",local_data.i,*local_data.count);
				pthread_mutex_unlock(&local_data.lock);
				child_stuff(&local_data);
				pthread_mutex_lock(&local_data.lock);
				count++; /* we need this count to be 10 in total from both the threads. */
				pthread_mutex_unlock(&local_data.lock);
			}

			res = pthread_join(a_thread, &thread_result); 
			if(res != 0) {
				error_exit("thread joined failed \n");
			}  
			printf("Thread joined, it returned\n  %s\n", (char *)thread_result);  
			pthread_mutex_destroy(&work_mutex[i]);
			return 0;/* safe exit */
		} 
	}

	while( count1 < 40 ) {
		printf("id =  %d\n",(int)getpid());
			readf = active; /* ok this is confusing because of the name as active and read */
			if ((status = select (FD_SETSIZE, &readf, NULL, NULL, NULL)) < 0) {
				printf("%s",strerror(errno));
				exit (EXIT_FAILURE);
			}
			for(i = 0; i < FD_SETSIZE; ++i) {
				if (FD_ISSET (i, &readf)) {
					memset(buf,0, sizeof(buf));
					if(read(i, buf, BUFSIZ)>0) {
						printf("%s\n",buf);
						count1++;
						if(status = write(fd1, buf, strlen(buf)))
							printf("%d\n",status);
					}
				}
			}

			waitpid(-1, &status, WNOHANG);
			if (WIFEXITED(status)) 
				printf("exited, status=%d **COUNT**%d\n", WEXITSTATUS(status), count1);
	
	}

	close(fd1);
	return 0;
}
