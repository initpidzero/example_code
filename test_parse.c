#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pthread_common.h"

int main (void) 
{
	int i;
	struct parameter param;

	for(i=0;i<3;i++) {
		param.params[i] = (char *)malloc(BUFSIZ);
	}
	param.filepath = (char *)malloc(BUFSIZ);
	strcpy(param.filepath, "deamon_param.cfg");
	parse_file(&param);

	score_debug("test = %s", param.params[0]);
	score_debug("test = %s", param.params[1]);
	
	free(param.filepath);
	free(param.params[0]);
	free(param.params[1]);
	free(param.params[2]);

	return 0;
}
