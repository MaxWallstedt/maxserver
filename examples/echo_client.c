/**
 * Copyright © 2018  Max Wällstedt <max.wallstedt@gmail.com>
 *
 * This file is part of maxserver.
 *
 * maxserver is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * maxserver is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with maxserver.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/select.h>

#include "client_socket.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

/**
 * Global variable holding the self pipe.
 */
static int self_pipe[2];

/**
 * Closes the self pipe.
 * On error, an appropriate error message is printed to standard
 * error.
 */
static void close_self_pipe()
{
	int err;

	/* Close read end of the self pipe. */
	err = close(self_pipe[0]);

	if (err == -1) {
		perror("close");
	}

	/* Close write end of the self pipe. */
	err = close(self_pipe[1]);

	if (err == -1) {
		perror("close");
	}
}

/**
 * Initialises the self pipe.
 * On success, zero is returned. On error, -1 is returned, and an
 * appropriate error message is printed to standard error.
 */
static int init_self_pipe()
{
	int err;

	/* Create self pipe. */
	err = pipe(self_pipe);

	if (err == -1) {
		perror("pipe");
		return -1;
	}

	/* Mark read end of the self pipe as non-blocking. */
	err = fcntl(self_pipe[0], F_SETFL, O_NONBLOCK);

	if (err == -1) {
		perror("fcntl");
		close_self_pipe();
		return -1;
	}

	/* Mark write end of the self pipe as non-blocking. */
	err = fcntl(self_pipe[1], F_SETFL, O_NONBLOCK);

	if (err == -1) {
		perror("fcntl");
		close_self_pipe();
		return -1;
	}

	return 0;
}

/**
 * On SIGPIPE, does nothing. On other signals, writes to the self
 * pipe.
 */
static void signal_handler(int signum)
{
	char c = 0;

	if (signum != SIGPIPE) {
		write(self_pipe[1], &c, 1);
	}
}

/**
 * Registers signal handler for all relevant signals.
 */
static int register_signal_handler()
{
	struct sigaction sa;
	int err;

	/* Initialise 'sa' data structure. */
	sa.sa_handler = signal_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	/* Register signal handler. */
	err = sigaction(SIGHUP, &sa, NULL);

	if (err == -1) {
		perror("sigaction");
		return -1;
	}

	err = sigaction(SIGINT, &sa, NULL);

	if (err == -1) {
		perror("sigaction");
		return -1;
	}

	err = sigaction(SIGQUIT, &sa, NULL);

	if (err == -1) {
		perror("sigaction");
		return -1;
	}

	err = sigaction(SIGABRT, &sa, NULL);

	if (err == -1) {
		perror("sigaction");
		return -1;
	}

	err = sigaction(SIGPIPE, &sa, NULL);

	if (err == -1) {
		perror("sigaction");
		return -1;
	}

	err = sigaction(SIGTERM, &sa, NULL);

	if (err == -1) {
		perror("sigaction");
		return -1;
	}

	err = sigaction(SIGTSTP, &sa, NULL);

	if (err == -1) {
		perror("sigaction");
		return -1;
	}

	return 0;
}

/**
 * Writes 'len' and 'input' to server through 'sfd', and reads 'len'
 * bytes from server through 'sfd' and prints them to standard
 * output.
 * On success, zero is returned. On error, -1 is returned, and an
 * appropriate error message is printed to standard error.
 */
static int echo_client_perform(int sfd, const char *input, size_t len)
{
	char *echo;
	ssize_t res;

	/* Write 'len' to server. */
	res = write(sfd, &len, sizeof(size_t));

	if (res == -1) {
		if (errno == EPIPE) {
			fprintf(
				stderr,
				"write: connection closed by server.\n"
			);
		} else {
			perror("write");
		}

		return -1;
	} else if ((size_t)res < sizeof(size_t)) {
		fprintf(stderr, "write: too few bytes written.\n");
		return -1;
	}

	/* Write 'input' to server. */
	res = write(sfd, input, len);

	if (res == -1) {
		if (errno == EPIPE) {
			fprintf(
				stderr,
				"write: connection closed by server.\n"
			);
		} else {
			perror("write");
		}

		return -1;
	} else if ((size_t)res < len) {
		fprintf(stderr, "write: too few bytes written.\n");
		return -1;
	}

	/* Allocate echo buffer. */
	echo = malloc(len + 1);

	if (echo == NULL) {
		perror("malloc");
		return -1;
	}

	/* Read server data. */
	res = read(sfd, echo, len);

	if (res == -1) {
		perror("read");
		free(echo);
		return -1;
	} else if (res == 0) {
		fprintf(stderr, "read: connection closed by server.\n");
		free(echo);
		return -1;
	} else if ((size_t)res < len) {
		fprintf(stderr, "read: too few bytes read.\n");
		free(echo);
		return -1;
	}

	echo[len] = '\0';

	/* Print server data. */
	fprintf(stdout, "%s\n", echo);

	free(echo);
	return 0;
}

/**
 * Reads from standard input until end-of-file is reached and calls
 * 'echo_client_perform'.
 * On success, zero is returned. On error, -1 is returned, and an
 * appropriate error message is printed to standard error.
 */
static int echo_client(int sfd)
{
	fd_set rfds;
	int maxfd;
	size_t len;
	size_t alloc, tmp_alloc;
	char *input, *tmp_input;
	int c;
	int err;

	/* Set up file descriptor set. */
	FD_ZERO(&rfds);
	FD_SET(STDIN_FILENO, &rfds);
	FD_SET(self_pipe[0], &rfds);
	FD_SET(sfd, &rfds);
	maxfd = MAX(STDIN_FILENO, MAX(self_pipe[0], sfd));

	/* Wait for input from either standard input, the self pipe
	   or server. */
	for (;;) {
		err = select(maxfd + 1, &rfds, NULL, NULL, NULL);

		if (err == -1) {
			/* If error is caused by interrupting signal,
			   try again. Otherwise, print error message
			   and break. */
			if (errno == EINTR) {
				continue;
			} else {
				perror("select");
				return -1;
			}
		} else if (FD_ISSET(self_pipe[0], &rfds)) {
			/* The self pipe has signalled the program to
			   quit. */
			return 0;
		} else if (FD_ISSET(sfd, &rfds)) {
			/* Server has closed the connection. */
			fprintf(stderr, "connection closed by server.\n");
			return -1;
		} else if (FD_ISSET(STDIN_FILENO, &rfds)) {
			/* Input is available from standard input. */
			break;
		}
	}

	/* Allocate input buffer. */
	len = 0;
	alloc = 128;
	input = malloc(alloc);

	if (input == NULL) {
		perror("malloc");
		return -1;
	}

	/* Read from standard input to input buffer until end-of-file
	   is reached. */
	while ((c = fgetc(stdin)) != EOF) {
		/* Ensure that input buffer can hold another byte. */
		if (len == alloc) {
			tmp_alloc = 2 * alloc;
			tmp_input = realloc(input, tmp_alloc);

			if (tmp_input == NULL) {
				perror("realloc");
				free(input);
				return -1;
			}

			alloc = tmp_alloc;
			input = tmp_input;
		}

		input[len] = c;
		++len;
	}

	/* Perform echo client. */
	err = echo_client_perform(sfd, input, len);

	if (err == -1) {
		free(input);
		return -1;
	}

	free(input);
	return 0;
}

int main(int argc, char *argv[])
{
	int sfd;
	int err;

	if (argc != 3) {
		fprintf(stderr, "usage: %s host port\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	/* Initialise the self pipe. */
	err = init_self_pipe();

	if (err == -1) {
		exit(EXIT_FAILURE);
	}

	/* Register signal handler. */
	err = register_signal_handler();

	if (err == -1) {
		close_self_pipe();
		exit(EXIT_FAILURE);
	}

	/* Create client socket. */
	sfd = client_socket(argv[1], argv[2]);

	if (sfd == -1) {
		close_self_pipe();
		exit(EXIT_FAILURE);
	}

	/* Perform echo client. */
	err = echo_client(sfd);

	if (err == -1) {
		close(sfd);
		close_self_pipe();
		exit(EXIT_FAILURE);
	}

	close(sfd);
	close_self_pipe();
	return 0;
}
