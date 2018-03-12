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

#ifndef CLIENT_THREAD_H
#define CLIENT_THREAD_H

/**
 * Initialises client threads data structures.
 * On success, zero is returned. On error, -1 is returned, and an
 * appropriate error message is printed to standard error.
 */
int client_threads_init();

/**
 * Clears client threads data structures.
 */
void client_threads_clear();

/**
 * Returns 1 if client threads have been signalled to quit. Returns 0
 * otherwise.
 */
int client_threads_should_quit();

/**
 * Starts client thread using client socket file descriptor 'cfd' and
 * calls 'client_thread'.
 * On success, zero is returned. On error, -1 is returned, and an
 * appropriate error message is printed to standard error.
 */
int client_thread_start(
	int cfd,
	void (*client_thread)(int cfd, int sigpipe),
	int sigpipe
);

/**
 * Stops all client threads.
 */
void client_threads_stop();

#endif
