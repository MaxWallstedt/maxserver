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

#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H

/**
 * Creates a TCP server socket on port 'service', ready to accept
 * incoming connections.
 * On success, a file descriptor for the new socket is returned. On
 * error, -1 is returned, and an appropriate error message is printed
 * to standard error.
 */
int server_socket(const char *service);

#endif
