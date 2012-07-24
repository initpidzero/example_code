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

void create_server(void)
{
	int status;
	int sd[MAX_PORT];
	int addrlen;
	const unsigned short int port[] = {0x1234, 0x1235, 0x1236, 0x1237};
	struct sockaddr_in sin[MAX_PORT];
	struct sockaddr_in pin[MAX_PORT];
	char buf[BUFSIZ];
	int fd1; 
	fd_set active_fd_set;
	int i = 0;	
	int j = 0;
	pid_t child_id[4];
	char msg[BUFSIZ];
	struct timeval timeout;

	fd1 = open ("/tmp/whatever.txt", O_CREAT |  O_RDWR | O_TRUNC, 0644);

	timeout.tv_sec  = 1 * 60;
	timeout.tv_usec = 0;

	/* Initialize the set of active sockets. */
	FD_ZERO (&active_fd_set);
	addrlen = sizeof(pin[0]); 


	system("echo begin > /tmp/error.txt"); 

	for(; j < MAX_PORT; j++) {

		populate_sock(&sd[j],&sin[j], port[j]);
			FD_SET (sd[j], &active_fd_set);
//			fcntl(sd[j], F_SETFL, O_NONBLOCK);

		child_id[j] = fork();
		if(child_id[j] < 0) {
			system("echo fork error, REALLY? >> /tmp/error.txt"); 
			goto out;
		} else if(child_id[j] == 0) {
			log_stuff("*****server side*****, child_process=", j);
			while(1) {
				if (select (FD_SETSIZE, &active_fd_set, NULL, NULL, &timeout) < 0) {
					log_stuff(strerror(errno), __LINE__);
					FD_CLR(sd[j], &active_fd_set);
					close(sd[j]);
					exit (EXIT_FAILURE);
				}

				if (FD_ISSET (sd[j], &active_fd_set)) {
					log_stuff("Really ready filedesc=", sd[j]);
					memset(buf, 0, sizeof(buf));
					if( recvfrom(sd[j],buf, sizeof(buf), 0, (struct sockaddr *)&pin[j], &addrlen) == -1 ) {
						log_stuff(strerror(errno), __LINE__);
						FD_CLR(sd[j], &active_fd_set);
						close(sd[j]);
						exit(EXIT_FAILURE);
					}
					log_stuff(buf,j);
					status = write(fd1, buf, strlen(buf));
					log_stuff("write status=",status);
					status = write(fd1, "\n", strlen("\n"));

					if(!strcmp(buf,"EOM")){
						log_stuff("EOM recieved on server thread=", j);
						FD_CLR(sd[j], &active_fd_set);
						close(sd[j]);
						exit(EXIT_SUCCESS);
					}
/*
					sprintf(buf, "%s%d","from_server_thread=",j);
					if(sendto(sd[j], buf, sizeof(buf), 0, (struct sockaddr *)&pin[j], addrlen) == -1 ) {
						log_stuff(strerror(errno), __LINE__);
						FD_CLR(sd[j], &active_fd_set);
						close(sd[j]);
						exit(EXIT_FAILURE);
					} */
					memset(buf, 0, sizeof(buf));
					if(recvfrom(sd[j],buf, sizeof(buf), 0, (struct sockaddr *)&pin[j], &addrlen) == -1){
						log_stuff(strerror(errno), __LINE__);
						FD_CLR(sd[j], &active_fd_set);
						close(sd[j]);
						exit(EXIT_FAILURE);
					}
					if(!strcmp(buf,"EOM")){
						log_stuff("EOM recieved on server thread=", j);
						FD_CLR(sd[j], &active_fd_set);
						close(sd[j]);
						exit(EXIT_SUCCESS);
					}

				}
			}
		} else {
			log_stuff("Server daddy process attempt", j);
			log_stuff("daddy process child id ", child_id[j]);

			waitpid(child_id[j], &status, 0);
			if(WIFEXITED(status))
				log_stuff("successful exit in server thread=", j);
			else
				log_stuff("unsuccessful exit in server thread=", j);
		}
			log_stuff("Server daddy, post waitpid", j);
	}

	log_stuff("before waitpid ", __LINE__);

out:
/*	for(j = 0; j < MAX_PORT; j++) {
		waitpid(child_id[j], &status, 0);
		if(WIFEXITED(status))
			log_stuff("successful exit in server thread=", j);
		else
			log_stuff("unsuccessful exit in server thread=", j);
	}
*/
	log_stuff("after waitpid line=", __LINE__);
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

int main (void) {
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
