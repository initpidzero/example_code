#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/semaphore.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/time.h>

#define MAX_PORT 4

extern int errno;
char SEM_NAME[]= "vik";

void log_stuff(const char *message, int parameter)
{
	char msg[BUFSIZ];
	
	memset(msg, 0, sizeof(msg));
	sprintf(msg, "echo %s %d >> /tmp/error.txt", message, parameter);
	system(msg);

}

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

void child_here(fd_set read_fd_set, int fd, sem_t *mutex )
{
	int status;
	int i;
	char buf[BUFSIZ];
	struct sockaddr_in pin;
	int addrlen = sizeof(pin); 
	pid_t child_id;
	fd_set active_fd_set;
       
	child_id = fork();


	if (child_id  == 0)
		while(1) {
			active_fd_set = read_fd_set; /* ok this is confusing because of the name as active and read */
			if ((status = select (FD_SETSIZE, &active_fd_set, NULL, NULL, NULL)) < 0) {
				log_stuff(strerror(errno), __LINE__);
				exit (EXIT_FAILURE);
			}

			for(i = 0; i < FD_SETSIZE; ++i)
				if (FD_ISSET (i, &active_fd_set)) {
					log_stuff("Really ready filedesc=", i);
					memset(buf, 0, sizeof(buf));
					if( recvfrom(i,buf, sizeof(buf), 0, (struct sockaddr *)&pin, &addrlen) == -1 ) {
						log_stuff(strerror(errno), __LINE__);
						FD_CLR(i, &active_fd_set);
						close(i);
						exit(EXIT_FAILURE);
					}

					sem_wait(mutex);
					log_stuff(buf,i);
					status = write(fd, buf, strlen(buf));
					log_stuff("write status=",status);
					status = write(fd, "\n", strlen("\n"));
					sem_post(mutex);

					if(!strcmp(buf,"EOM")){
						log_stuff("EOM recieved on server thread=", i);
						FD_CLR(i, &active_fd_set);
						close(i);
						exit(EXIT_SUCCESS);
					}
		}
	}

}

void create_server(void)
{
	int status;
	int sd[MAX_PORT];
	int addrlen;
	const unsigned short int port[] = {0x1234, 0x1235, 0x1236, 0x1237};
	struct sockaddr_in sin[MAX_PORT];
	int fd1; 
	int j = 0;
	pid_t child_id[4];
	struct timeval timeout;
	pid_t parent_id = getpid();
	sem_t *mutex;
	fd_set read_fd_set;
	fd1 = open ("/tmp/whatever.txt", O_CREAT |  O_RDWR | O_TRUNC, 0644);

	timeout.tv_sec  = 1 * 60;
	timeout.tv_usec = 0;

	/* Initialize the set of active sockets. */
	FD_ZERO (&read_fd_set);


	mutex = sem_open(SEM_NAME,O_CREAT,0644,1);
	if(mutex == SEM_FAILED)
	{
		log_stuff("unable to create semaphore",errno);
		sem_unlink(SEM_NAME);
		exit(EXIT_FAILURE);
	}

	system("echo begin > /tmp/error.txt"); 

	for(; j < MAX_PORT; j++) {
		populate_sock(&sd[j],&sin[j], port[j]);

//		fcntl(sd[j], F_SETFL, O_NONBLOCK);
		log_stuff("filedesc we get=", sd[j]);
		FD_SET (sd[j], &read_fd_set);
		child_here(read_fd_set, fd1, mutex );
	}


	if(getpid() == parent_id)
		for(j = 0; j < MAX_PORT; j++) {
		log_stuff("******Before Waitpid ",(int)getpid());
			waitpid(-1, &status, 0);
			if(WIFEXITED(status))
				log_stuff("successful exit in server thread=", j);
			else
				log_stuff("unsuccessful exit in server thread=", j);
		}

	log_stuff("after waitpid line=", __LINE__);

	sem_close(mutex);
	sem_unlink(SEM_NAME);
	exit(EXIT_FAILURE);
}

void deamonify(void)
{
	pid_t sid;
	int fd;
	unsigned int i = 100;

	umask(027);
	sid = setsid(); /* get a new process group. */
	chdir("/");
	umask(0);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	fd = open("/dev/null", O_RDWR);/* stdin */
	(void) dup(fd);  /* stdout */
	(void) dup(fd);  /* stderr */
       	printf("again  process = %d\n ",getpid());
	create_server();
}

int main (void) 
{

	pid_t processId;
	if ((processId = fork()) == 0) {
		printf("child process = %d %d \n ",getpid(), __LINE__);
		deamonify();
	} else if (processId < 0) {
		perror("fork error");
	} else {
		printf("parent process = %d\n ",getpid());
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}
