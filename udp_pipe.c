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

void child_here(fd_set read_fd_set, int fd, sem_t *mutex, int pd)
{
	int status;
	int i;
	char buf[BUFSIZ];
	struct sockaddr_in pin;
	int addrlen = sizeof(pin); 
	pid_t child_id;
	fd_set active_fd_set;

	FD_ZERO (&active_fd_set);

	child_id = fork();

	if (child_id  == 0) {
		int count = 0; 
		while(1) {
		//	active_fd_set = read_fd_set; /* ok this is confusing because of the name as active and read */
		/*	if ((status = select (FD_SETSIZE, &active_fd_set, NULL, NULL, NULL)) < 0) {
				log_stuff(strerror(errno), __LINE__);
				exit (EXIT_FAILURE);
			}

			for(i = 0; i < FD_SETSIZE; ++i) {
				if (FD_ISSET (i, &active_fd_set)) {
					log_stuff("ready filedesc in child = ", i);
					memset(buf, 0, sizeof(buf));
					if( recvfrom(i,buf, sizeof(buf), 0, (struct sockaddr *)&pin, &addrlen) == -1 ) {
						log_stuff(strerror(errno), __LINE__);
						FD_CLR(i, &active_fd_set);
						close(i);
						exit(EXIT_FAILURE);
					}
					log_stuff("port",ntohs(pin.sin_port));
					sem_wait(mutex);
					log_stuff(buf,i); */
			sprintf(buf,"pid=%d,count=%d",getpid(), count);
					status = write(pd, buf, strlen(buf));
					log_stuff("write status in child=",status);
					if(status > 0) {
						count++;
					}
					if(count == 10){
						exit(EXIT_SUCCESS);
					}
				/*	sem_post(mutex);

					if(!strcmp(buf,"EOM")){
						log_stuff("EOM recieved on child server thread=", i);
						FD_CLR(i, &active_fd_set);
						close(i);
						exit(EXIT_SUCCESS);
					}
				}
			} */
		} /* while */
	}

}

void parent_stuff(fd_set pipe_fd_set, int fd1, sem_t *mutex)
{
	int count = 0;
	fd_set pipe_active_set; /* currently active pipe set */
	int i;
	char buf[MAX_PORT];
	int status;

	FD_ZERO (&pipe_active_set);
	while(count < 40) {
		pipe_active_set = pipe_fd_set; /* ok this is confusing because of the name as active and read */
		if ((status = select (FD_SETSIZE, &pipe_active_set, NULL, NULL, NULL)) < 0) {
			log_stuff(strerror(errno), __LINE__);
			exit (EXIT_FAILURE);
		}
		for(i = 0; i < FD_SETSIZE; ++i)
			if (FD_ISSET (i, &pipe_active_set)) {
				log_stuff("ready filedesc in parent = ", i);
				memset(buf, 0, sizeof(buf));
				status = read(i, buf, strlen(buf));
				log_stuff("read status in parent = ", status);
				if(status > 0) {
					status = write(fd1, buf, strlen(buf));
					log_stuff("write status in parent = ", status);
					status = write(fd1, "\n", strlen("\n"));
		//			if(!strcmp(buf,"EOM")){
		//				log_stuff("EOM recieved on server thread=", i);
		//				FD_CLR(i, &pipe_active_set);
		//				close(i);
						count++;
		//			}

				}
			}

		waitpid(-1, &status, WNOHANG);
		if(WIFEXITED(status))
			log_stuff("successful exit in server thread=", count);
		else
			log_stuff("unsuccessful exit in server thread=", count);
	}

}

void create_server(void)
{
	int status;
	int sd[MAX_PORT];
	int addrlen;
	const unsigned short int port[] = {0x9234, 0x9235, 0x9236, 0x9237}; 
	struct sockaddr_in sin[MAX_PORT];
	int fd1; 
	int j = 0;
	int i = 0;
	pid_t child_id[4];
//	struct timeval timeout;
	pid_t parent_id = getpid();
	sem_t *mutex;
	fd_set read_fd_set;
	fd_set pipe_fd_set; /* pipe set */
	int pd[MAX_PORT][2]; /* pipes */
	char buf[MAX_PORT];
	int count = 0;

	fd1 = open ("/tmp/whatever.txt", O_CREAT |  O_RDWR | O_TRUNC, 0644);

//	timeout.tv_sec  = 1 * 60;
//	timeout.tv_usec = 0;

	/* Initialize the set of active sockets. */
	FD_ZERO (&read_fd_set);
	FD_ZERO (&pipe_fd_set);


	mutex = sem_open(SEM_NAME, O_CREAT, 0644, 1);
	if(mutex == SEM_FAILED)
	{
		log_stuff("unable to create semaphore",errno);
		sem_unlink(SEM_NAME);
		exit(EXIT_FAILURE);
	}

	system("echo begin > /tmp/error.txt"); 

	for(; j < MAX_PORT; j++) {

		populate_sock(&sd[j],&sin[j], port[j]);
		if(pipe(pd[j]) == -1) {
			log_stuff(strerror(errno), __LINE__);
			exit(EXIT_FAILURE);		
		}
//		fcntl(sd[j], F_SETFL, O_NONBLOCK);
		log_stuff("filedesc we get=", sd[j]);
		FD_SET (sd[j], &read_fd_set);
		FD_SET (pd[j][0], &pipe_fd_set); /* */
		child_here(read_fd_set, fd1, mutex, pd[j][1]);/* */
	}


	if(getpid() == parent_id) {
		parent_stuff(pipe_fd_set, fd1, mutex);
	}

	log_stuff("after waitpid line=", getpid());
	close(fd1);
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
