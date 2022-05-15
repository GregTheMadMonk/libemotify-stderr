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
	char c; // Character that will be read
	int newline = 1; // Is set to 1 each time the '\n' character is encountered
			 // and indicates that it is needed to print a reaction
	const char* react = reactions[0]; // Next reaction to be printed

	// Read pipe input character-by-character
	while (read(pipe_fds[0], &c, 1) != 0) {
		if (c == '\n') { // New line
			newline = 1;
		} else if (newline && isprint(c)) {
			// Print an emoji after a new line if something is about to get printed
			newline = 0;
			// Get a random reaction from the list and print it
			react = reactions[rand_range(lenof(reactions))];
			write(stderr_old, react, strlen(react));
		}
		write(stderr_old, &c, 1); // Pass output through
	}
	pthread_exit(NULL);
}

void libemotify_init(void) {
	printf("My PID: %d\n", getpid());
	// Initialize RNG
	srand(time(0));

	// Create a pipe to pass stderr through
	pipe(pipe_fds);
	// Create a copy of the original stderr file descriptor
	stderr_old = dup(STDERR_FILENO);
	// Reassign pipe input to stderr
	dup2(pipe_fds[1], STDERR_FILENO);

	// Create a thread to read process stderr
	pthread_create(&reader_thread, NULL, watchdog, NULL);
}

void libemotify_fini(void) {
	// Close the fake stderr
	close(pipe_fds[1]);
	// Return stderr to its original state
	dup2(stderr_old, STDERR_FILENO);
	// Wait for the output to finish
	pthread_join(reader_thread, NULL);
	// Close the pipe output
	close(pipe_fds[0]);
}
