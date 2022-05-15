#define _GNU_SOURCE
#include <ctype.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define lenof(array) sizeof(array)/sizeof(*array)
#define rand_range(range) rand() * range / RAND_MAX

static int pipe_fds[2];// Pipe I/O file descriptors
static int stderr_old;	// Old stderr file descriptor
static pthread_t reader_thread; // Thread for reading the pipe

static const char* reactions[] = {
	" 😳  ",
	" 😕  ",
	" 🤔  ",
	" 😡  ",
};

static void* watchdog(void*) {
	char c;
	int newline = 1;
	const char* react = reactions[0];
	while (read(pipe_fds[0], &c, 1) != 0) {
		if (c == '\n') {
			newline = 1;
		} else if (newline && isprint(c)) {
			newline = 0;
			react = reactions[rand_range(lenof(reactions))];
			write(stderr_old, react, strlen(react));
		}
		write(stderr_old, &c, 1);
	}
	pthread_exit(NULL);
}

void libemotify_init(void) {
	srand(time(0));
	pipe(pipe_fds); // Open a pipe for stderr
	stderr_old = dup(STDERR_FILENO);
	dup2(pipe_fds[1], STDERR_FILENO);

	pthread_create(&reader_thread, NULL, watchdog, NULL);
}

void libemotify_fini(void) {
	write(pipe_fds[1], "\0", 1);
	close(pipe_fds[1]);
	pthread_join(reader_thread, NULL);
	close(pipe_fds[0]);
}
