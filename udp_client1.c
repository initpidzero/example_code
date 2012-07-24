#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

/* REPLACE with your server machine name*/
#define HOST        "127.0.0.1"
#define DIRSIZE     8192
#define MAX_PORT     4

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
        char hostname[100];

       	strcpy(hostname,HOST);

	if ((*sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	memset(sin, 0, sizeof(*sin));
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = inet_addr(hostname);
	sin->sin_port = htons(port);

}

int main(int argc, char **argv) 
{
	char dir[BUFSIZ];
	int sd[MAX_PORT],j=0;
	struct sockaddr_in sin[MAX_PORT];
	const unsigned short int port[] = {0x9234, 0x9235, 0x9236, 0x9237}; 
	pid_t child_id[MAX_PORT];
	int addrlen;
	int status;
	
	if(!argv[1]) argv[1] = "from_client=";
	addrlen = sizeof(sin[0]); 

	for(; j < MAX_PORT; j++) {

		populate_sock(&sd[j],&sin[j], port[j]);
		printf("file_des= %d\n",sd[j]);
		child_id[j] = fork();
		if(child_id[j] < 0) {
			printf("fork error, REALLY? \n"); 
			exit(EXIT_FAILURE);
		} else if(child_id[j] == 0) {

			printf("%s,%d\n","====>Client side, In child=", port[j]);
			sprintf(dir, "%s%d",argv[1],j);
			/* send a message to the server PORT on machine HOST */
			if ((status  = sendto(sd[j], dir, strlen(dir), 0, (struct sockaddr *)&sin[j], addrlen))== -1) {
				printf("%s %d",strerror(errno), __LINE__);
				exit(1);
			}
			printf("%s,%d\n","status=", status);

			/* send a message to the server PORT on machine HOST */
			if ((status = sendto(sd[j], "EOM", strlen("EOM"), 0, (struct sockaddr *)&sin[j], addrlen))== -1) {
				printf("%s %d",strerror(errno), __LINE__);
				exit(1);
			}
			printf("%s,%d\n","status=", status);

			/* spew-out the results and bail out of here! */

			exit(EXIT_SUCCESS);
		} else {
			printf("=====>Client side daddy \n");	

			waitpid(child_id[j], &status, 0);
			if(WIFEXITED(status))
				printf("Hellalujah\n");
			else
				printf("What in the name of devil!\n");

			printf("for Client child %d \n", child_id[j]);	

			close(sd[j]);
		}	
		sleep(1);
	}	



	return 0;
}
