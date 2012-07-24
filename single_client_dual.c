#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

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
	struct sockaddr_in sin, pin;
	int addrlen;
	int status;
	const unsigned short int port;	
	time_t epoch_time;

	sscanf(argv[1], "%x", &port);

	addrlen = sizeof(sin); 


	populate_sock(&sd,&sin, port);
	epoch_time = time((time_t)0); 
	printf("%ld",epoch_time);
	sprintf(dir,"%ld %s",epoch_time ,argv[1]);
	/* send a message to the server PORT on machine HOST */
	if ((status  = sendto(sd, dir, strlen(dir), 0, (struct sockaddr *)&sin, addrlen))== -1) {
		printf("%s %d",strerror(errno), __LINE__);
		exit(1);
	}
	printf("%s,%d\n","status=", status);

	memset(dir, 0, sizeof(dir));
	/* wait for a message to come back from the server */
	if (recvfrom(sd, dir, DIRSIZE, 0, (struct sockaddr *)&pin, &addrlen) == -1) {
		printf("%s %d",strerror(errno), __LINE__);
		exit(1);
	}

	printf("message from server = %s\n", dir);

	memset(dir, 0, sizeof(dir));
	sprintf(dir, "%ld %s",epoch_time ,"EOM");
	/* send a message to the server PORT on machine HOST */
	if ((status = sendto(sd, dir, strlen(dir), 0, (struct sockaddr *)&sin, addrlen))== -1) {
		printf("%s %d",strerror(errno), __LINE__);
		exit(1);
	}
	printf("%s,%d\n","status=", status);

	memset(dir, 0, sizeof(dir));
	/* wait for a message to come back from the server */
	if (recvfrom(sd, dir, DIRSIZE, 0, (struct sockaddr *)&pin, &addrlen) == -1) {
		printf("%s %d",strerror(errno), __LINE__);
		exit(1);
	}

	printf("message from server = %s\n", dir);



	return 0;
}
