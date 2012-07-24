#include <semaphore.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#define SHMSZ 27
char SEM_NAME[]= "vik";

/* main function: shoot rockets 
 * @ argc int
 * @ argv char**
 *
 * returns value to shell ;
 */

 int main(int argc, char **argv)
{
	int fd, i,count=0,nloop=10,zero=0,*ptr;
	sem_t *mutex;

	//open a file and map it into memory

	fd = open("log.txt",O_RDWR|O_CREAT,S_IRWXU);
	write(fd,&zero,sizeof(int));
	ptr = mmap(NULL,sizeof(int),PROT_READ |PROT_WRITE,MAP_SHARED,fd,0);
	close(fd);

	mutex = sem_open(SEM_NAME,O_CREAT,0644,1);
	if(mutex == SEM_FAILED)
	{
		perror("unable to create semaphore");
		sem_unlink(SEM_NAME);
		exit(-1);
	}

	if (fork() == 0) { /* child process*/
		for (i = 0; i < nloop; i++) {
			sem_wait(mutex);
			printf("child: %d\n", (*ptr)++);
			sem_post(mutex);
		}
		exit(0);
	}
	/* back to parent process */
	for (i = 0; i < nloop; i++) {
		sem_wait(mutex);
		printf("parent: %d\n", (*ptr)++);
		sem_post(mutex);
	}

	sem_close(mutex);
	sem_unlink(SEM_NAME);

	exit(0);
}
