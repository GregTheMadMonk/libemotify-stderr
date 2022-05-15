#define _GNU_SOURCE
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define BUF_SIZE 2048
#define lenof(array) sizeof(array)/sizeof(*array)

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
	char c[BUF_SIZE];
	size_t len;
	const char** react = reactions;
	while ((len = read(pipe_fds[0], c, BUF_SIZE)) != 0) {
		react = &reactions[rand() * lenof(reactions) / RAND_MAX];
		write(stderr_old, *react, strlen(*react));
		write(stderr_old, c, len);
	}
	pthread_exit(NULL);
}

void _init(void) {
	srand(time(0));
	pipe(pipe_fds); // Open a pipe for stderr
	stderr_old = dup(STDERR_FILENO);
	dup2(pipe_fds[1], STDERR_FILENO);

	pthread_create(&reader_thread, NULL, watchdog, NULL);
}

void _fini(void) {
	close(pipe_fds[1]);
	pthread_join(reader_thread, NULL);
	close(pipe_fds[0]);
}
