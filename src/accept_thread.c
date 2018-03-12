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

#include "accept_thread.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "print_error.h"
#include "client_thread.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

/**
 * Data structure representing the accept thread argument.
 */
struct accept_thread_arg {
	int sfd;
	int sigpipe;
	void (*client_thread)(int cfd, int sigpipe);
};

/**
 * Global variable holding the thread ID of accept thread.
 */
static pthread_t accept_thread_id;

/**
 * Calls 'client_thread' on every incoming client connection until
 * accept thread is signalled to quit.
 */
static void accept_thread(
	int sfd,
	int sigpipe,
	void (*client_thread)(int cfd, int sigpipe)
)
{
	fd_set rfds, rfds_copy;
	int maxfd;
	struct sockaddr_storage addr;
	socklen_t addrlen;
	char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
	int cfd;
	int err;

	/* Initialise 'rfds' and add 'sfd' and signal pipe. */
	FD_ZERO(&rfds);
	FD_ZERO(&rfds_copy);
	FD_SET(sfd, &rfds);
	FD_SET(sigpipe, &rfds);
	maxfd = MAX(sfd, sigpipe);

	for (;;) {
		rfds_copy = rfds;
		err = select(maxfd + 1, &rfds_copy, NULL, NULL, NULL);

		if (err == -1) {
			if (errno == EINTR) {
				continue;
			}

			print_error_errno("accept_thread:select");
			return;
		}

		if (FD_ISSET(sigpipe, &rfds_copy)) {
			break;
		}

		/* Accept client connection. */
		addrlen = sizeof(struct sockaddr_storage);
		cfd = accept(sfd, (struct sockaddr *)&addr, &addrlen);

		if (cfd == -1) {
			print_error_errno("accept_thread:accept");
			continue;
		}

		/* Get name information of client. */
		err = getnameinfo(
			(struct sockaddr *)&addr,
			addrlen,
			hbuf,
			NI_MAXHOST,
			sbuf,
			NI_MAXSERV,
			0
		);

		if (err != 0) {
			print_error_gai("accept_thread:getnameinfo", err);
			close(cfd);
			continue;
		}

		/* Print name information of client. */
		fprintf(
			stdout,
			"accepted connection from %s:%s\n",
			hbuf,
			sbuf
		);

		/* Start client thread. */
		err = client_thread_start(cfd, client_thread, sigpipe);

		if (err == -1) {
			close(cfd);
			continue;
		}
	}
}

/**
 * Calls accept thread and manages dynamically allocated data 'arg'.
 */
static void *accept_thread_starter(void *arg)
{
	struct accept_thread_arg *at_arg;

	at_arg = (struct accept_thread_arg *)arg;
	accept_thread(at_arg->sfd, at_arg->sigpipe, at_arg->client_thread);
	free(at_arg);
	pthread_exit(NULL);
}

/**
 * Starts accept thread using server socket file descriptor 'sfd',
 * and calls 'client_thread' on every incoming client connection.
 * On success, zero is returned. On error, -1 is returned, and an
 * appropriate error message is printed to standard error.
 */
int accept_thread_start(
	int sfd,
	int sigpipe,
	void (*client_thread)(int cfd, int sigpipe)
)
{
	struct accept_thread_arg *at_arg;
	int err;

	/* Initialise client threads data structures. */
	err = client_threads_init();

	if (err == -1) {
		return -1;
	}

	/* Allocate accept thread argument data structure. */
	at_arg = malloc(sizeof(struct accept_thread_arg));

	if (at_arg == NULL) {
		print_error_errno("accept_thread_start:malloc");
		client_threads_stop();
		client_threads_clear();
		return -1;
	}

	at_arg->sfd = sfd;
	at_arg->sigpipe = sigpipe;
	at_arg->client_thread = client_thread;

	/* Start accept thread. */
	err = pthread_create(
		&accept_thread_id,
		NULL,
		accept_thread_starter,
		at_arg
	);

	if (err != 0) {
		print_error("accept_thread_start:pthread_create", err);
		free(at_arg);
		client_threads_stop();
		client_threads_clear();
		return -1;
	}

	return 0;
}

/**
 * Stops accept thread, and any thread spawned by accept thread.
 */
void accept_thread_stop()
{
	int err;

	/* Wait for accept thread to quit. */
	err = pthread_join(accept_thread_id, NULL);

	if (err != 0) {
		print_error("accept_thread_stop:pthread_join", err);
	}

	/* Stop client threads. */
	client_threads_stop();

	/* Clear client threads data structures. */
	client_threads_clear();
}
