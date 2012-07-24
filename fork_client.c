#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>

#define PORT        0x1234
             /* REPLACE with your server machine name*/
#define HOST        "127.0.0.1"
#define DIRSIZE     8192

int main(int argc, char **argv) 
{
        char hostname[100];
	char dir[BUFSIZ];
	int	sd,i;
	struct sockaddr_in sin;
	struct sockaddr_in pin;
	struct hostent *hp;

        strcpy(hostname,HOST);
        if (argc>2)
            { strcpy(hostname,argv[2]); }

	/* fill in the socket structure with host information */
	memset(&pin, 0, sizeof(pin));
	pin.sin_family = AF_INET;
//	pin.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr))->s_addr;
	pin.sin_addr.s_addr = inet_addr(hostname);
	pin.sin_port = htons(PORT);

	/* grab an Internet domain socket */
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	/* connect to PORT on HOST */
	if (connect(sd,(struct sockaddr *)  &pin, sizeof(pin)) == -1) {
		perror("connect");
		exit(1);
	}

	//for()
	/* send a message to the server PORT on machine HOST */
	if (send(sd, argv[1], strlen(argv[1]), 0) == -1) {
		perror("send");
		exit(1);
	}

        /* wait for a message to come back from the server */
        if (recv(sd, dir, DIRSIZE, 0) == -1) {
                perror("recv");
                exit(1);
        }

        printf("%s\n", dir);

	if (send(sd, "send2", strlen("send2"), 0) == -1) {
		perror("send");
		exit(1);
	}

        /* wait for a message to come back from the server */
        if (recv(sd, dir, DIRSIZE, 0) == -1) {
                perror("recv");
                exit(1);
        }

	if (send(sd, "EOM", strlen("EOM"), 0) == -1) {
		perror("send");
		exit(1);
	}

        /* spew-out the results and bail out of here! */
        printf("%s\n", dir);

	close(sd);
	
	return 0;
}
