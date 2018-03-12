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

#include "maxserver.h"

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/select.h>

#include "print_error.h"
#include "server_socket.h"
#include "accept_thread.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

/**
 * Global variable holding the signal pipe.
 */
static int maxserver_sigpipe[2];

/**
 * Global variable holding the server's socket file descriptor.
 */
static int maxserver_sfd;

/**
 * Clears any data held by the server.
 */
static void maxserver_clear()
{
	accept_thread_stop();
	close(maxserver_sigpipe[0]);
	close(maxserver_sigpipe[1]);
	close(maxserver_sfd);
}

/**
 * Function that is called when SIGINT is raised. Signals the program
 * to quit.
 */
static void maxserver_signal_handler(int signum)
{
	char sig = 0;

	if (signum == SIGINT) {
		write(maxserver_sigpipe[1], &sig, 1);
		fputc('\n', stdout);
	}
}

/**
 * Registers 'maxserver_signal_handler' to be called when SIGINT is
 * raised.
 * On success, zero is returned. On error, -1 is returned, and an
 * appropriate error message is printed to standard error.
 */
static int maxserver_register_signal_handler()
{
	struct sigaction sa;
	int err;

	/* Initialise 'sa' data structure. */
	sa.sa_handler = maxserver_signal_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	/* Register signal handler. */
	err = sigaction(SIGINT, &sa, NULL);

	if (err == -1) {
		print_error_errno(
			"maxserver_register_signal_handler:sigaction"
		);
		return -1;
	}

	return 0;
}

/**
 * Starts the server on port 'service', and calls 'client_thread' on
 * every incoming client connection. This function blocks until
 * SIGINT is raised or end-of-file is read from standard input.
 * On success, zero is returned. On error, -1 is returned, and an
 * appropriate error message is printed to standard error.
 */
int maxserver(
	const char *service,
	void (*client_thread)(int cfd, int sigpipe)
)
{
	fd_set rfds, rfds_copy;
	int maxfd;
	char sig = 0;
	int err;

	/* Create TCP server socket. */
	maxserver_sfd = server_socket(service);

	if (maxserver_sfd == -1) {
		return -1;
	}

	/* Set up signal pipe. */
	err = pipe(maxserver_sigpipe);

	if (err == -1) {
		print_error_errno("maxserver:pipe");
		close(maxserver_sfd);
		return -1;
	}

	/* Mark signal pipe as non-blocking. */
	err = fcntl(maxserver_sigpipe[0], F_SETFL, O_NONBLOCK);

	if (err == -1) {
		print_error_errno("maxserver:fcntl");
		close(maxserver_sigpipe[0]);
		close(maxserver_sigpipe[1]);
		close(maxserver_sfd);
		return -1;
	}

	err = fcntl(maxserver_sigpipe[1], F_SETFL, O_NONBLOCK);

	if (err == -1) {
		print_error_errno("maxserver:fcntl");
		close(maxserver_sigpipe[0]);
		close(maxserver_sigpipe[1]);
		close(maxserver_sfd);
		return -1;
	}

	/* Initialise 'rfds' and add signal pipe and standard input. */
	FD_ZERO(&rfds);
	FD_ZERO(&rfds_copy);
	FD_SET(maxserver_sigpipe[0], &rfds);
	FD_SET(STDIN_FILENO, &rfds);
	maxfd = MAX(maxserver_sigpipe[0], STDIN_FILENO);

	/* Register signal handler. */
	err = maxserver_register_signal_handler();

	if (err == -1) {
		close(maxserver_sigpipe[0]);
		close(maxserver_sigpipe[1]);
		close(maxserver_sfd);
		return -1;
	}

	/* Start accept thread. */
	err = accept_thread_start(
		maxserver_sfd,
		maxserver_sigpipe[0],
		client_thread
	);

	if (err == -1) {
		close(maxserver_sigpipe[0]);
		close(maxserver_sigpipe[1]);
		close(maxserver_sfd);
		return -1;
	}

	/* Read from standard input until end-of-file is read or the
	   signal pipe receives input. */
	for (;;) {
		rfds_copy = rfds;
		err = select(
			maxfd + 1,
			&rfds_copy,
			NULL,
			NULL,
			NULL
		);

		if (err == -1) {
			if (errno == EINTR) {
				continue;
			}

			print_error_errno("maxserver:select");
			maxserver_clear();
			return -1;
		}

		if (FD_ISSET(maxserver_sigpipe[0], &rfds_copy)) {
			break;
		} else if (FD_ISSET(STDIN_FILENO, &rfds_copy)) {
			if (fgetc(stdin) == EOF) {
				write(maxserver_sigpipe[1], &sig, 1);
				break;
			}
		}
	}

	/* Clear any data held by the server. */
	maxserver_clear();

	return 0;
}
