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
	int sd,j=0;
	struct sockaddr_in sin;
	int addrlen;
	int status;
	const unsigned short int port;	

	sscanf(argv[1], "%x", &port);

	addrlen = sizeof(sin); 


	populate_sock(&sd,&sin, port);

	sprintf(dir, "%s%d",argv[1],j);
	/* send a message to the server PORT on machine HOST */
	if ((status  = sendto(sd, dir, strlen(dir), 0, (struct sockaddr *)&sin, addrlen))== -1) {
		printf("%s %d",strerror(errno), __LINE__);
		exit(1);
	}
	printf("%s,%d\n","status=", status);

	/* send a message to the server PORT on machine HOST */
	if ((status = sendto(sd, "EOM", strlen("EOM"), 0, (struct sockaddr *)&sin, addrlen))== -1) {
		printf("%s %d",strerror(errno), __LINE__);
		exit(1);
	}
	printf("%s,%d\n","status=", status);


	return 0;
}
