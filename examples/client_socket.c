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

#include "client_socket.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

/**
 * Creates a TCP client socket connected to host 'node' and port
 * 'service'.
 * On success, a file descriptor for the new socket is returned. On
 * error, -1 is returned, and an appropriate error message is printed
 * to standard error.
 */
int client_socket(const char *node, const char *service)
{
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int sfd;
	int err;

	/* Initialise 'hints' data structure. */
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_flags = AI_ADDRCONFIG;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;

	/* Get 'addrinfo' data structures. */
	err = getaddrinfo(node, service, &hints, &result);

	if (err != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
		return -1;
	}

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		/* Create socket. */
		sfd = socket(
			rp->ai_family,
			rp->ai_socktype,
			rp->ai_protocol
		);

		if (sfd == -1) {
			continue;
		}

		/* Connect socket to address. */
		err = connect(sfd, rp->ai_addr, rp->ai_addrlen);

		if (err == -1) {
			close(sfd);
			continue;
		}

		/* Socket creation successful. */
		break;
	}

	/* Free 'result', since it's no longer needed. */
	freeaddrinfo(result);

	if (rp == NULL) {
		fprintf(
			stderr,
			"client_socket: "
			"could not create client socket.\n"
		);
		return -1;
	}

	return sfd;
}
