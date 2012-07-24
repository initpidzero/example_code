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

#define PORT 0x1234

char SEM_NAME[]= "vik";

extern int errno;

void log_stuff(const char *message, int parameter)
{
	char msg[BUFSIZ];
	
	memset(msg, 0, sizeof(msg));
	sprintf(msg, "echo %s %d >> /tmp/error.txt", message, parameter);
	system(msg);

}

void create_server(void)
{
	int status;
	int 	 sd, sd_current;
	int 	 addrlen;
	struct   sockaddr_in sin;
	struct   sockaddr_in pin;
	char buf[BUFSIZ];
	int fd1; 
//	sem_t *mutex;
	fd_set active_fd_set, read_fd_set;
	int i = 0;	
	int j = 0;
	pid_t child_id[4];
	int child_des[4];
	char msg[BUFSIZ];

	fd1 = open ("/tmp/whatever.txt", O_CREAT |  O_RDWR | O_TRUNC, 0644);


	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	/* Initialize the set of active sockets. */
	FD_ZERO (&active_fd_set);
	FD_SET (sd, &active_fd_set);
/*
	mutex = sem_open(SEM_NAME,O_CREAT,0644,1);
	if(mutex == SEM_FAILED)
	{
		perror("unable to create semaphore");
		sem_unlink(SEM_NAME);
		exit(EXIT_FAILURE);
	}

*/	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(PORT);

	if (bind(sd, (struct sockaddr *) &sin, sizeof(sin)) == -1) {
		perror("bind");
		goto out;
	}

	if (listen(sd, 5) == -1) {
		perror("listen");
		goto out;
	}

	addrlen = sizeof(pin); 

	system("echo begin > /tmp/error.txt"); 

	for(; j < 4; j++) {
		child_id[j] = fork();
		if(child_id < 0) {
			system("echo fork error, REALLY? >> /tmp/error.txt"); 
			goto out;
		} else if(child_id[j] == 0) {
			log_stuff("In child on attempt", j);
			if ((child_des[j] = accept(sd, (struct sockaddr *)  &pin, &addrlen)) == -1) {
				system("echo accept error >> /tmp/error.txt"); 
				perror("accept");
				goto out;
			}
			log_stuff("In child with desc=", child_des[j]);
			FD_SET (child_id[j], &active_fd_set);
			//sleep(5);
			exit(EXIT_SUCCESS);
		} else {
			log_stuff("daddy process child des=", child_des[j]);

		}

	}

	while(1) {
		/* Block until input arrives on one or more active sockets. */
		//if(j < 4){
		//	child_id[j] = fork();
		//}
		read_fd_set = active_fd_set;
		if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0) {
			system("select error >> /tmp/error.txt"); 
			perror ("select");
			exit (EXIT_FAILURE);
		}

		/* Service all the sockets with input pending. */
		for (i = 0; i < FD_SETSIZE; ++i) {
			if (FD_ISSET (i, &read_fd_set)) {
				log_stuff("select desc",i);
				//	if (i == sd) {
				//		system("echo new_connection >> /tmp/error.txt"); 
				/* Connection request on original socket. */
				/*		if ((sd_current = accept(sd, (struct sockaddr *)  &pin, &addrlen)) == -1) {
						system("echo accept error >> /tmp/error.txt"); 
						perror("accept");
						goto out;
						}
						FD_SET (sd_current, &active_fd_set);
						} else {
						*/		
				system("echo rec_connection >> /tmp/error.txt"); 
				/* Data arriving on an already-connected socket. */
				if (recv(i, buf, sizeof(buf), 0) == -1) {
					log_stuff(strerror(errno), i);
					perror("recv");
					close (i);
					FD_CLR (i, &active_fd_set);
					goto out;
				}
				status = write(fd1, buf, strlen(buf));
				status = write(fd1, "\n", strlen("\n"));

				if(!strcmp(buf,"EOM")){
					system("echo EOM recieved >> /tmp/error.txt"); 
					close (i);
					FD_CLR (i, &active_fd_set);
					continue;
				}

				system("echo sen_connection >> /tmp/error.txt"); 
				if (send(i, buf, strlen(buf), 0) == -1) {
					system("echo send error >> /tmp/error.txt"); 
					perror("send");
					goto out;
				}

			}
		}	

		/*	
					
		} */

	waitpid(-1, &status, WNOHANG);
	if(WIFEXITED(status))
		system("echo successful exit >> /tmp/exit.txt"); 

	}
out:
	close(sd);
	close(sd_current);
	//sem_close(mutex);
	//sem_unlink(SEM_NAME);
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
