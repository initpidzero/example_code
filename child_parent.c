#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/semaphore.h>
#include <netinet/in.h>

#define PORT 0x1234

char SEM_NAME[]= "vik";

create_server()
{
	int status;
	int 	 sd, sd_current;
	int 	 addrlen;
	struct   sockaddr_in sin;
	struct   sockaddr_in pin;
	char buf[BUFSIZ];
	int fd1; 
	sem_t *mutex;
	
	fd1 = open ("/tmp/whatever.txt", O_CREAT |  O_RDWR, 0644);

	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	mutex = sem_open(SEM_NAME,O_CREAT,0644,1);
	if(mutex == SEM_FAILED)
	{
		perror("unable to create semaphore");
		sem_unlink(SEM_NAME);
		exit(EXIT_FAILURE);
	}

	memset(&sin, 0, sizeof(sin));
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

	while(1) {
		pid_t child_id;
		if ((sd_current = accept(sd, (struct sockaddr *)  &pin, &addrlen)) == -1) {
			system("echo accept error >> /tmp/error.txt"); 
			perror("accept");
			goto out;
		}
		child_id = fork();

		if(child_id < 0) {
			system("fork error, REALLY? >> /tmp/error.txt"); 
			goto out;
		} else if(child_id == 0){
			/* processing in child */
			if (recv(sd_current, buf, sizeof(buf), 0) == -1) {
				system("echo recv error >> /tmp/error.txt"); 
				perror("recv");
				goto out;
			}

			sem_wait(mutex);
			status = write(fd1, buf, strlen(buf));
			status = write(fd1, "\n", strlen("\n"));
			sem_post(mutex);

			if (send(sd_current, buf, strlen(buf), 0) == -1) {
				system("echo send error >> /tmp/error.txt"); 
				perror("send");
				goto out;
			}

			exit(EXIT_SUCCESS);
		} else {
			/* continue looping */
			close(sd_current);
			sleep(1);
			waitpid(-1, &status, WNOHANG);
			if(WIFEXITED(status))
				system("echo successful exit >> /tmp/success.txt"); 
					
		}
	}
out:
	close(sd);
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
