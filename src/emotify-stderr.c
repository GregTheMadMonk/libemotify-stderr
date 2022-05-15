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

static int use_reacts = 1;
static const char reaction_abort = '\0';
static const char* reactions[] = {
	"😳 ", "😕 ", "🤔 ", "😡 ", "🤣 ", "🙃 ", "🥲 ",
	"😋 ", "🤫 ", "🤨 ", "😐 ", "🤮 ", "🤯 ", "😵 ",
	"🤓 ", "🧐 ", "🥺 ", "😖 ", "💀 ", "🙀 ", "🙊 ",
};

static int use_colors = 0;
static const char* colors_reset = "\033[0m";
static const char* colors[] = {
	"1;31", // Red
	"1;35", // Magenta
};

static void* watchdog(void*) {
	char c; // Character that will be read
	int newline = 1; // Is set to 1 each time the '\n' character is encountered
			 // and indicates that it is needed to print a reaction
	const char* react = reactions[0]; // Next reaction to be printed
	const char* color = colors[0]; // Next output color

	// Read pipe input character-by-character
	while (read(pipe_fds[0], &c, 1) != 0) {
		if (newline && (c == reaction_abort)) {
			// If '\n' is followed by reaction_abort, the reaction output is aborted
			newline = 0;
			continue;
		}

		// Each time a printable character is written after a newline, output a reaction
		if (c == '\n') {
			if (use_colors) write(stderr_old, colors_reset, strlen(colors_reset));
			newline = 1;
		} else if (newline && isprint(c)) {
			newline = 0;
			if (use_reacts) {
				// Get a random reaction from the list and print it
				react = reactions[rand_range(lenof(reactions))];
				// Abort next reactions
				// This prevents a chain of reactions when output is
				// invoked by a child process
				write(stderr_old, &reaction_abort, 1);
				// Print a reaction
				write(stderr_old, react, strlen(react));
			}

			if (use_colors) {
				// Write a random color code
				color = colors[rand_range(lenof(colors))];
				write(stderr_old, "\033[", 2);
				write(stderr_old, color, strlen(color));
				write(stderr_old, "m", 1);
			}
		}
		write(stderr_old, &c, 1); // Pass original output through
	}
	pthread_exit(NULL);
}

void libemotify_init(void) {
	// Initialize RNG
	srand(time(0));

	// Get environment
	const char* env_colors = getenv("EMOTIFY_COLORS");
	if (env_colors) use_colors = !strcmp(env_colors, "1");
	const char* env_reacts = getenv("EMOTIFY_REACTS");
	if (env_reacts) use_reacts = strcmp(env_reacts, "0");

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
