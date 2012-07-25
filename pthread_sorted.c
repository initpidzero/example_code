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

#include "pthread_common.h"

#define MAX_PORT 4
#define THREAD_COUNT 2
char master_of_the_buffer [20][BUFSIZ]; /* obey your masters, your life burns faster */ 
extern int errno;
short unsigned int ports  = 0x9123; /* higher port starting from 37155*/ 
int current = 0;
struct element *list, *head;
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
//	anuz_debug("number of descriptor %d\n", status);
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
	time_t epoch_time = time((time_t*)0); 

	memset(buf, 0, sizeof(buf));
	if( recvfrom(sock , buf, sizeof(buf), 0, (struct sockaddr *)&pin, &addrlen) == -1 ) {
		printf("recvfrom error");
	}

	strcpy(input, buf);
//	anuz_debug("%s\n",buf);

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%ld %s",epoch_time ,"from_server");
	if ( sendto(sock, buf , strlen(buf), 0, (struct sockaddr *)&pin, addrlen) == -1) {
		printf("%s %d",strerror(errno), __LINE__);
		exit(1);
	}
}

void child_stuff( struct local_s *local)
{
	memset(local->input, 0, sizeof(local->input));
	get_me_the_fds(local->active, &local->sock );
	get_me_the_buffer(local->sock, local->input, local->pin);
	
	pthread_mutex_lock(&local->lock);
	//	printf(" holding lock at %d %d %d\n", __LINE__, *local->count, local->i);
	//strcpy(local->input, "paapi");
	strcat(local->input, local->input1); 
	write(local->fd_pipe,local->input, sizeof(local->input));
	pthread_mutex_unlock(&local->lock);
	//	printf("lock released at %d %d %d\n", __LINE__, *local->count, local->i);

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
		//printf(" holding lock at %d %d %d\n", __LINE__, *local->count, local->i);
		memset(local->input1, 0, sizeof(local->input1));
		sprintf(local->input1, "__In_Thread-child-%d,count-%d\n",local->i,*local->count);
		pthread_mutex_unlock(&local->lock);
		//anuz_debug("lock released at %d %d %d\n", __LINE__, *local->count, local->i);

		child_stuff(local);

		pthread_mutex_lock(&local->lock);
	//	printf(" holding lock at %d %d %d\n", __LINE__, *local->count, local->i);
		(*count)++; /*if at all this works */
		pthread_mutex_unlock(&local->lock);
	//	printf("lock released at %d %d %d\n", __LINE__, *local->count, local->i);

	}

	pthread_exit("Thank you for the CPU time");
}

void get_config(int *thread_count,int *child_count)
{
	struct parameter param;

	param.params[0] = (char *)malloc(BUFSIZ);
	param.params[1] = (char *)malloc(BUFSIZ);
	param.filepath = (char *)malloc(BUFSIZ);

	strcpy(param.filepath, "deamon_param.cfg");
	parse_file(&param);

	*thread_count = atoi(param.params[0]);
	*child_count = atoi(param.params[1]);

	free(param.filepath);
	param.filepath = NULL;
	free(param.params[0]);
	param.params[0] = NULL;  
	free(param.params[1]);
	param.params[1] = NULL;  
}

void add_buf_to_list(char *buf)
{
	struct element *temp = NULL;

	temp = (struct element *)malloc(sizeof(struct element));
	temp->count = current;
	temp->string = (char *)malloc(strlen(buf)+1);
	strcpy(temp->string, buf);
	temp->next = NULL;

	if(!head) {
		list = temp;
		head = list;
		anuz_debug("list reinstialized =  %d\n",head->count);
	} else {
		list->next = temp;
		list = list->next;
	}
}

void write_to_file(int fd1, char *buf, int limit) 
{
	int i;
	int status;
	struct element *temp;

	add_buf_to_list(buf);
	anuz_debug("%d\n",current);
	//strcpy(master_of_the_buffer[current], buf);
	current++;
	if(current == limit) {
		head = sort_list(head);
//		merge_sort(master_of_the_buffer, current);
//		for(i = 0; i < limit; i++) {
		for(temp = head; temp; temp = temp->next){ 
			if(status = write(fd1, temp->string, strlen(temp->string)))
				anuz_debug("counter = %d status = %d line=%d\n",temp->count, status, __LINE__);
		}
		 write(fd1, "===8===\n", strlen("===8===\n"));
		 current = 0;
		 free_list(head);
		 head = list = NULL;
	} 

}

void create_server(void)
	//void demonize(void) 
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
	struct timeval timeout;
	static pthread_mutex_t work_mutex[MAX_PORT]; /* the lock this file needs */
	int thread_count, child_count;
	fd1 = open ("/tmp/whatever.txt", O_CREAT |  O_RDWR | O_TRUNC, 0644);
	//anuz_debug("parent id %d\n",(int)getpid());

	FD_ZERO(&active);
	FD_ZERO(&readf);
	FD_ZERO(&active_sd);

	timeout.tv_sec  = 1;
	timeout.tv_usec = 60;

	get_config(&thread_count, &child_count);
	printf("thread count %d\n",thread_count);
	printf("child count %d\n",child_count);

	for( ;i < child_count; i++) {
		pipe(pd[i]);

		//fcntl(pd[i][0], F_SETFL, O_NONBLOCK);
		FD_SET(pd[i][0], &active);
		s1[i].port = ports + i; 

		populate_sock(&s1[i].sd, &s1[i].sin, s1[i].port);
		FD_SET(s1[i].sd, &active_sd);

		pid[i] = fork();
		if(pid[i] == 0) {
			struct local_s local_data;
			int res;
			int j;
			pthread_t a_thread[thread_count];
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

			for(j = 0; j < thread_count; j++) {
				res = pthread_create(&a_thread[j], NULL, thread_function, (void *)&local_data); 
				if(res != 0) {
					error_exit("thread creation error \n");
				} 
			}
			while(count < 10) {

				pthread_mutex_lock(&local_data.lock);
				//				anuz_debug(" lock holding in child at %d %d %d\n", __LINE__, count, i);
				memset(local_data.input1, 0, sizeof(local_data.input1));
				sprintf(local_data.input1, "__In_Main-child-%d,count-%d\n",local_data.i,*local_data.count);
				pthread_mutex_unlock(&local_data.lock);
				//				anuz_debug(" lock released in child at %d %d %d\n", __LINE__, count, i);

				child_stuff(&local_data);

				pthread_mutex_lock(&local_data.lock);
				//				printf(" lock holding in child at %d %d %d\n", __LINE__, count, i);
				count++; /* we need this count to be 10 in total from both the threads. */
				pthread_mutex_unlock(&local_data.lock);
				//				anuz_debug(" lock released in child at %d %d %d\n", __LINE__, count, i);

			}

			for(j = 0; j < thread_count; j++) {
				res = pthread_join(a_thread[j], &thread_result); 
				if(res != 0) {
					error_exit("thread joined failed \n");
				}  
				anuz_debug("Thread joined, it returned\n  %s\n", (char *)thread_result);  
			}
			pthread_mutex_destroy(&work_mutex[i]);
			exit(EXIT_SUCCESS);/* safe exit */
		} 
	}

	while( count1 < 10 * child_count ) {
		anuz_debug("id =  %d\n",(int)getpid());
		readf = active; /* ok this is confusing because of the name as active and read */
		if ((status = select (FD_SETSIZE, &readf, NULL, NULL, NULL)) < 0) {
			printf("%s",strerror(errno));
			exit (EXIT_FAILURE);
		}
		for(i = 0; i < FD_SETSIZE; ++i) {
			if (FD_ISSET (i, &readf)) {
				memset(buf,0, sizeof(buf));
				if(read(i, buf, BUFSIZ)>0) {
					anuz_debug("%s\n",buf);
					count1++;
					write_to_file(fd1, buf, child_count * thread_count);
				}
			}
		}

		waitpid(-1, &status, WNOHANG);
		if (WIFEXITED(status)) 
			anuz_debug("exited, status=%d **COUNT**%d\n", WEXITSTATUS(status), count1);

	}

	close(fd1);
}

void deamonify(void)
{
	pid_t sid;
	int fd;

	sid = setsid(); /* get a new process group. */
	chdir("/");
	umask(0);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	fd = open("/dev/null", O_RDWR);/* stdin */
	(void) dup(fd);  /* stdout */
	(void) dup(fd);  /* stderr */
	anuz_debug ("again  process = %d\n ",getpid());
	create_server();
}

int demonize() 
{

	pid_t pid;
	if ((pid = fork()) == 0) {
		anuz_debug("child process = %d %d \n ",getpid(), __LINE__);
		deamonify();
	} else if (pid < 0) {
		fprintf(stderr,"fork error %s\n", strerror(errno));
	} else {
		anuz_debug("parent process = %d\n ",getpid());
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}

int main(void) 
{
	//return	demonize();
	create_server();
	return 0;
}
