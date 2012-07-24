#ifndef __PTHREAD_COMMON_H__
#define __PTHREAD_COMMON_H__

#define score_debug printf

struct parameter {
	char *params[3];
	char *filepath;
};

struct element {
	struct element *next, *prev;
	int count;
	char *string;
};

int parse_file(struct parameter * param);

void merge_sort (char x[][BUFSIZ], int n);

void free_list(struct element *list);

struct element * sort_list(struct element *list);

#endif /* __PTHREAD_COMMON_H__ */
