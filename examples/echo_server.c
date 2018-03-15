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
#include <sys/select.h>

#include <maxserver.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

/**
 * Reads length of client data and client data from client through
 * 'cfd', and prints it to standard output and writes it to client
 * through 'cfd'.
 * On error, an appropriate error message is printed to standard
 * error.
 */
static void echo_server_perform(int cfd)
{
	char *echo;
	size_t len;
	ssize_t res;

	/* Read length of client data. */
	res = read(cfd, &len, sizeof(size_t));

	if (res == -1) {
		perror("read");
		return;
	} else if (res == 0) {
		fprintf(stderr, "read: connection closed by client.\n");
		return;
	} else if ((size_t)res < sizeof(size_t)) {
		fprintf(stderr, "read: too few bytes read.\n");
		return;
	}

	/* Allocate echo buffer. */
	echo = malloc(len + 1);

	if (echo == NULL) {
		perror("malloc");
		return;
	}

	/* Read client data. */
	res = read(cfd, echo, len);

	if (res == -1) {
		perror("read");
		free(echo);
		return;
	} else if (res == 0) {
		fprintf(stderr, "read: connection closed by client.\n");
		free(echo);
		return;
	} else if ((size_t)res < len) {
		fprintf(stderr, "read: too few bytes read.\n");
		free(echo);
		return;
	}

	echo[len] = '\0';

	/* Print client data. */
	fprintf(stdout, "%s\n", echo);

	/* Write client data to client. */
	res = write(cfd, echo, len);

	if (res == -1) {
		if (errno == EPIPE) {
			fprintf(
				stderr,
				"write: connection closed by client.\n"
			);
		} else {
			perror("write");
		}

		free(echo);
		return;
	} else if ((size_t)res < len) {
		fprintf(stderr, "write: too few bytes written.\n");
		free(echo);
		return;
	}

	free(echo);
}

/**
 * Waits for input from either 'cfd' or 'sigpipe', and calls
 * 'echo_server_perform' if 'cfd' has input first.
 * On error, an appropriate error message is printed to standard
 * error.
 */
static void echo_server(int cfd, int sigpipe)
{
	fd_set rfds;
	int maxfd;
	int err;

	/* Set up file descriptor set. */
	FD_ZERO(&rfds);
	FD_SET(cfd, &rfds);
	FD_SET(sigpipe, &rfds);
	maxfd = MAX(cfd, sigpipe);

	/* Wait for input from either 'cfd' or 'sigpipe'. */
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
				break;
			}
		} else if (FD_ISSET(sigpipe, &rfds)) {
			/* 'sigpipe' has signalled the thread to
			   quit. */
			break;
		} else if (FD_ISSET(cfd, &rfds)) {
			/* Input is available from 'cfd', perform
			   echo server. */
			echo_server_perform(cfd);
			break;
		}
	}
}

int main(int argc, char *argv[])
{
	int err;

	if (argc != 2) {
		fprintf(stderr, "usage: %s port\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	/* Start server with 'echo_server' as the client thread. */
	err = maxserver(argv[1], echo_server);

	if (err == -1) {
		exit(EXIT_FAILURE);
	}

	return 0;
}
