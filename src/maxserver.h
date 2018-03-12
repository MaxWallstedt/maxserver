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

#ifndef MAXSERVER_H
#define MAXSERVER_H

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
);

#endif
