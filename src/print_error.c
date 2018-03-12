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

#include "print_error.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

/**
 * Prints an error message to standard error corresponding to error
 * number 'err'. If 'prefix' is not NULL and 'prefix[0]' is not '\0',
 * it will be printed before the error message followed by a colon
 * and a space.
 */
void print_error(const char *prefix, int err)
{
	if (prefix != NULL && prefix[0] != '\0') {
		fprintf(stderr, "%s: ", prefix);
	}

	fprintf(stderr, "%s\n", strerror(err));
}

/**
 * Prints an error message to standard error corresponding to
 * 'getaddrinfo' error number 'err'. If 'prefix' is not NULL and
 * 'prefix[0]' is not '\0', it will be printed before the error
 * message followed by a colon and a space.
 */
void print_error_gai(const char *prefix, int err)
{
	if (prefix != NULL && prefix[0] != '\0') {
		fprintf(stderr, "%s: ", prefix);
	}

	fprintf(stderr, "%s\n", gai_strerror(err));
}

/**
 * Prints an error message to standard error corresponding to error
 * number 'errno'. If 'prefix' is not NULL and 'prefix[0]' is not
 * '\0', it will be printed before the error message followed by a
 * colon and a space.
 */
void print_error_errno(const char *prefix)
{
	print_error(prefix, errno);
}

/**
 * Prints error message 'str' to standard error. If 'prefix' is not
 * NULL and 'prefix[0]' is not '\0', it will be printed before the
 * error message followed by a colon and a space.
 */
void print_error_str(const char *prefix, const char *str)
{
	if (prefix != NULL && prefix[0] != '\0') {
		fprintf(stderr, "%s: ", prefix);
	}

	fprintf(stderr, "%s\n", str);
}
