#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>
#include <errno.h>

#define PORT 		0x1234
#define DIRSIZE 	8192
#define DIR_SIZE 8192

void read_dir(char *dir)
{
	DIR * dirp;
	struct dirent *d;

	/* open directory */
	dirp = opendir(dir);
	if (dirp == NULL)
		return;

	/* stuff filenames into dir buffer */
	dir[0] = '\0';
	while (d = readdir(dirp))
		sprintf(dir, "%s%s\n", dir, d->d_name);
	printf("%s",dir);
	/* return the result */
	closedir(dirp);
}

void write_to_file(char *dir)
{

	int status;
	int fd = open ("/tmp/whatever.txt", O_CREAT |  O_RDWR , 0644);

	status = write(fd, dir, strlen(dir));
	if(status == -1)
		perror("write failed");

	close(fd);
}

int child_process()
{
	char     dir[DIRSIZE];  /* used for incomming dir name, and
				   outgoing data */
	int 	 sd, sd_current, cc, fromlen, tolen;
	int 	 addrlen;
	struct   sockaddr_in sin;
	struct   sockaddr_in pin;
	printf("In child process %d %s \n",__LINE__, __FUNCTION__ );

	/* get an internet domain socket */
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	/* complete the socket structure */
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(PORT);

	/* bind the socket to the port number */
	if (bind(sd, (struct sockaddr *) &sin, sizeof(sin)) == -1) {
		perror("bind");
		goto out;
	}

	/* show that we are willing to listen */
	if (listen(sd, 5) == -1) {
		perror("listen");
		goto out;
	}
	/* wait for a client to talk to us */
	addrlen = sizeof(pin); 
	while(1) {
		if ((sd_current = accept(sd, (struct sockaddr *)  &pin, &addrlen)) == -1) {
			perror("accept");
			goto out;
		}
		/* if you want to see the ip address and port of the client, uncomment the 
		 *     next two lines */


		printf("Hi there, from  %s#\n",inet_ntoa(pin.sin_addr));
		printf("Coming from port %d\n",ntohs(pin.sin_port));


		/* get a message from the client */
		if (recv(sd_current, dir, sizeof(dir), 0) == -1) {
			perror("recv");
			goto out;
		}

		/* get the directory contents */
		//	read_dir(dir);

		write_to_file(dir);
		/* strcat (dir," DUDE");
		 *        */
		/* acknowledge the message, reply w/ the file names */
		if (send(sd_current, dir, strlen(dir), 0) == -1) {
			perror("send");
			goto out;
		}

		/* close up both sockets */
		close(sd_current);
		sleep(1);
	}

	out:
	close(sd);
	exit(EXIT_FAILURE);

}

void daemonify(void)
{
	pid_t pid, sid;
	int fd;
	if((pid = fork()) < 0) {
		exit(EXIT_FAILURE);
	} else if(pid != 0) {
		printf("parent pid = %d\n", pid);
		/* close the parent process */
		sleep(1);
		exit(EXIT_SUCCESS);
	}
	printf("%d",__LINE__);
	umask(027);
	printf("%d",__LINE__);
	sid = setsid(); /* get a new process group. */

	printf("%d",__LINE__);
	if(sid < 0) {
		/*failure to deamonify*/
		exit(EXIT_FAILURE);
	}

	printf("%d",__LINE__);
	chdir("/");
	umask(0);

	printf("%d",__LINE__);
	/* Close out the standard file descriptors */
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	printf("%d",__LINE__);
	fd = open("/dev/null", O_RDWR);/* stdin */
	(void) dup(fd);  /* stdout */
	(void) dup(fd);  /* stderr */

	printf("child pid = %d\n", pid);
	child_process();

}

void main(void)
{

	printf( "current pid %d\n", (int)getpid());
	daemonify();

	printf( "current pid post demonization %d\n", (int)getpid());
	printf( "parent pid post demonization %d\n", (int)getppid());

	/* give client a chance to properly shutdown */

}
