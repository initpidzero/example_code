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

int main(void) 
{
	int pd[4][2];
	int pid[4];
	char buf[BUFSIZ];
	char input[BUFSIZ];
	int i = 0;
	int status;
	fd_set active;
	fd_set readf;
	int count = 0, count1 = 0;

	printf("parent id %d\n",(int)getpid());

	FD_ZERO(&active);
	FD_ZERO(&readf);

	
	for(;i <4;i++){
		pipe(pd[i]);
		FD_SET(pd[i][0], &active);
		pid[i] = fork();
		if(pid[i] == 0) {
			while(count < 10) {
				sprintf(input, "child-%d,count-%d",i,count);
				write(pd[i][1],input, sizeof(input));
				count++;
			}
			return 0;
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
					}
				}
			}

			waitpid(-1, &status, WNOHANG);
			if (WIFEXITED(status)) 
				printf("exited, status=%d **COUNT**%d\n", WEXITSTATUS(status), count1);
	

	return 0;
}
