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

#include "client_thread.h"

#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#include "print_error.h"

#define CLIENT_THREADS_ALLOC_INIT 64

/**
 * Data structure representing a client thread.
 */
struct client_thread {
	pthread_t tid;
	int finished;
};

/**
 * Data structure representing the client thread argument.
 */
struct client_thread_arg {
	pthread_t tid;
	int cfd;
	void (*client_thread)(int cfd, int sigpipe);
	int sigpipe;
};

/**
 * Forward declaration of function to remove 'tid' from client
 * threads array.
 */
static void client_threads_remove(pthread_t tid);

/**
 * Global variable holding client threads pipe.
 */
static int client_threads_pipe[2];

/**
 * Global variable holding client threads mutex lock.
 */
static pthread_mutex_t client_threads_lock;

/**
 * Global variable holding client threads array.
 */
static struct client_thread *client_threads;

/**
 * Global variable holding client threads array length.
 */
static size_t client_threads_len = 0;

/**
 * Global variable holding client threads array allocated length.
 */
static size_t client_threads_alloc = CLIENT_THREADS_ALLOC_INIT;

/**
 * Global variable holding the thread ID of client thread joiner.
 */
static pthread_t client_thread_joiner_id;

/**
 * Finds and removes finished client thread from client threads array.
 */
static void client_thread_joiner_perform()
{
	pthread_t tid;
	int err;
	size_t i;

	/* Obtain client threads mutex lock. */
	err = pthread_mutex_lock(&client_threads_lock);

	if (err != 0) {
		print_error(
			"client_thread_joiner_perform:pthread_mutex_lock",
			err
		);
		return;
	}

	/* Find finished client thread in client threads array. */
	for (i = 0; i < client_threads_len; ++i) {
		if (client_threads[i].finished == 1) {
			break;
		}
	}

	if (i == client_threads_len) {
		/**
		 * No client thread is finished in client threads array.
		 */

		/* Release client threads mutex lock. */
		err = pthread_mutex_unlock(&client_threads_lock);

		if (err != 0) {
			print_error(
				"client_thread_joiner_perform:"
				"pthread_mutex_unlock",
				err
			);
		}

		return;
	}

	/* Save ID of finished client thread. */
	tid = client_threads[i].tid;

	/* Release client threads mutex lock. */
	err = pthread_mutex_unlock(&client_threads_lock);

	if (err != 0) {
		print_error(
			"client_thread_joiner_perform:pthread_mutex_unlock",
			err
		);
	}

	/* Join finished client thread. */
	err = pthread_join(tid, NULL);

	if (err != 0) {
		print_error(
			"client_thread_joiner_perform:pthread_join",
			err
		);
	}

	/* Remove finished client thread from client threads array. */
	client_threads_remove(tid);
}

/**
 * Joins client threads that signal that they are finished.
 */
static void *client_thread_joiner(void *arg __attribute__((unused)))
{
	char c;
	ssize_t n_read;

	for (;;) {
		/* Read byte from client threads pipe. */
		n_read = read(client_threads_pipe[0], &c, 1);

		if (n_read == -1) {
			/* If error is caused by interrupting signal,
			   try again. */
			if (errno == EINTR) {
				continue;
			}

			/* If error is caused by anything else, break. */
			print_error_errno("client_thread_joiner:read");
			break;
		} else if (n_read == 0) {
			/* If end-of-file was reached, break. */
			break;
		}

		if (c == 0) {
			client_thread_joiner_perform();
		} else if (c == 1) {
			/* If client thread joiner was signalled to
			   quit, break. */
			break;
		}
	}

	pthread_exit(NULL);
}

/**
 * Initialises client threads data structures.
 * On success, zero is returned. On error, -1 is returned, and an
 * appropriate error message is printed to standard error.
 */
int client_threads_init()
{
	int err;

	/* Initialise client threads pipe. */
	err = pipe(client_threads_pipe);

	if (err == -1) {
		print_error_errno("client_threads_init:pipe");
		return -1;
	}

	/* Initialise client threads mutex lock. */
	err = pthread_mutex_init(&client_threads_lock, NULL);

	if (err != 0) {
		print_error("client_threads_init:pthread_mutex_init", err);

		/* Clear client threads pipe. */
		err = close(client_threads_pipe[0]);

		if (err == -1) {
			print_error_errno("client_threads_init:close");
		}

		err = close(client_threads_pipe[1]);

		if (err == -1) {
			print_error_errno("client_threads_init:close");
		}

		return -1;
	}

	/* Initialise client threads array. */
	client_threads = malloc(
		sizeof(struct client_thread) * client_threads_alloc
	);

	if (client_threads == NULL) {
		print_error_errno("client_threads_init:malloc");

		/* Clear client threads mutex lock. */
		err = pthread_mutex_destroy(&client_threads_lock);

		if (err != 0) {
			print_error(
				"client_threads_init:"
				"pthread_mutex_destroy",
				err
			);
		}

		/* Clear client threads pipe. */
		err = close(client_threads_pipe[0]);

		if (err == -1) {
			print_error_errno("client_threads_init:close");
		}

		err = close(client_threads_pipe[1]);

		if (err == -1) {
			print_error_errno("client_threads_init:close");
		}

		return -1;
	}

	/* Start client thread joiner. */
	err = pthread_create(
		&client_thread_joiner_id,
		NULL,
		client_thread_joiner,
		NULL
	);

	if (err != 0) {
		print_error("client_threads_init:pthread_create", err);

		/* Clear client threads array. */
		free(client_threads);

		/* Clear client threads mutex lock. */
		err = pthread_mutex_destroy(&client_threads_lock);

		if (err != 0) {
			print_error(
				"client_threads_init:"
				"pthread_mutex_destroy",
				err
			);
		}

		/* Clear client threads pipe. */
		err = close(client_threads_pipe[0]);

		if (err == -1) {
			print_error_errno("client_threads_init:close");
		}

		err = close(client_threads_pipe[1]);

		if (err == -1) {
			print_error_errno("client_threads_init:close");
		}

		return -1;
	}

	return 0;
}

/**
 * Clears client threads data structures.
 */
void client_threads_clear()
{
	int err;

	/* Clear client threads array. */
	free(client_threads);

	/* Clear client threads mutex lock. */
	err = pthread_mutex_destroy(&client_threads_lock);

	if (err != 0) {
		print_error(
			"client_threads_clear:pthread_mutex_destroy",
			err
		);
	}

	/* Clear client threads pipe. */
	err = close(client_threads_pipe[0]);

	if (err == -1) {
		print_error_errno("client_threads_clear:close");
	}

	err = close(client_threads_pipe[1]);

	if (err == -1) {
		print_error_errno("client_threads_clear:close");
	}
}

/**
 * Adds 'tid' to client threads array.
 * On success, zero is returned. On error, -1 is returned, and an
 * appropriate error message is printed to standard error.
 */
static int client_threads_add(pthread_t tid)
{
	struct client_thread *tmp_array;
	size_t tmp_alloc;
	int err;

	/* Obtain client threads mutex lock. */
	err = pthread_mutex_lock(&client_threads_lock);

	if (err != 0) {
		print_error("client_threads_add:pthread_mutex_lock", err);
		return -1;
	}

	/**
	 * Ensure that there is enough space for one more element in
	 * client threads array.
	 */

	if (client_threads_len == client_threads_alloc) {
		tmp_alloc = client_threads_alloc * 2;
		tmp_array = realloc(
			client_threads,
			sizeof(struct client_thread) * tmp_alloc
		);

		if (tmp_array == NULL) {
			print_error_errno("client_threads_add:realloc");

			/* Release client threads mutex lock. */
			err = pthread_mutex_unlock(&client_threads_lock);

			if (err != 0) {
				print_error(
					"client_threads_add:"
					"pthread_mutex_unlock",
					err
				);
			}

			return -1;
		}

		client_threads = tmp_array;
		client_threads_alloc = tmp_alloc;
	}

	/* Insert 'tid' into client threads array. */
	client_threads[client_threads_len].tid = tid;
	client_threads[client_threads_len].finished = 0;

	/* Update length of client threads array. */
	++client_threads_len;

	/* Release client threads mutex lock. */
	err = pthread_mutex_unlock(&client_threads_lock);

	if (err != 0) {
		print_error(
			"client_threads_add:pthread_mutex_unlock",
			err
		);
	}

	return 0;
}

/**
 * Marks 'tid' as finished in client threads array.
 */
static void client_threads_finish(pthread_t tid)
{
	char sig = 0;
	int err;
	size_t i;

	/* Obtain client threads mutex lock. */
	err = pthread_mutex_lock(&client_threads_lock);

	if (err != 0) {
		print_error(
			"client_threads_finish:pthread_mutex_lock",
			err
		);
		return;
	}

	/* Find 'tid' in client threads array. */
	for (i = 0; i < client_threads_len; ++i) {
		if (client_threads[i].tid == tid) {
			break;
		}
	}

	if (i == client_threads_len) {
		/**
		 * 'tid' is not in client threads array.
		 */

		/* Release client threads mutex lock. */
		err = pthread_mutex_unlock(&client_threads_lock);

		if (err != 0) {
			print_error(
				"client_threads_finish:"
				"pthread_mutex_unlock",
				err
			);
		}

		return;
	}

	/* Mark 'tid' as finished. */
	client_threads[i].finished = 1;

	/* Signal client thread joiner to join this thread. */
	write(client_threads_pipe[1], &sig, 1);

	/* Release client threads mutex lock. */
	err = pthread_mutex_unlock(&client_threads_lock);

	if (err != 0) {
		print_error(
			"client_threads_finish:pthread_mutex_unlock",
			err
		);
	}
}

/**
 * Removes 'tid' from client threads array.
 */
static void client_threads_remove(pthread_t tid)
{
	int err;
	size_t i;

	/* Obtain client threads mutex lock. */
	err = pthread_mutex_lock(&client_threads_lock);

	if (err != 0) {
		print_error(
			"client_threads_remove:pthread_mutex_lock",
			err
		);
		return;
	}

	/**
	 * Remove 'tid' from client threads array.
	 */

	/* Find 'tid' in client threads array. */
	for (i = 0; i < client_threads_len; ++i) {
		if (client_threads[i].tid == tid) {
			break;
		}
	}

	if (i == client_threads_len) {
		/**
		 * 'tid' is not in client threads array.
		 */

		/* Release client threads mutex lock. */
		err = pthread_mutex_unlock(&client_threads_lock);

		if (err != 0) {
			print_error(
				"client_threads_remove:"
				"pthread_mutex_unlock",
				err
			);
		}

		return;
	}

	/* Move elements after 'tid' one step to the left in client
	   threads array. */
	for (; i < client_threads_len - 1; ++i) {
		client_threads[i].tid = client_threads[i + 1].tid;
		client_threads[i].finished = client_threads[i + 1].finished;
	}

	/* Update length of client threads array. */
	--client_threads_len;

	/* Release client threads mutex lock. */
	err = pthread_mutex_unlock(&client_threads_lock);

	if (err != 0) {
		print_error(
			"client_threads_remove:pthread_mutex_unlock",
			err
		);
	}
}

/**
 * Calls client thread and manages dynamically allocated data 'arg'.
 */
static void *client_thread_starter(void *arg)
{
	struct client_thread_arg *ct_arg;
	int err;

	ct_arg = (struct client_thread_arg *)arg;

	/* Insert 'ct_arg->tid' into client threads array. */
	err = client_threads_add(ct_arg->tid);

	if (err == -1) {
		close(ct_arg->cfd);
		free(ct_arg);
		pthread_exit(NULL);
	}

	/* Call client thread. */
	ct_arg->client_thread(ct_arg->cfd, ct_arg->sigpipe);

	/* Mark 'ct_arg->tid' as finished in client threads array. */
	client_threads_finish(ct_arg->tid);

	close(ct_arg->cfd);
	free(ct_arg);
	pthread_exit(NULL);
}

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
)
{
	struct client_thread_arg *ct_arg;
	int err;

	/* Allocate client thread argument data structure. */
	ct_arg = malloc(sizeof(struct client_thread_arg));

	if (ct_arg == NULL) {
		print_error_errno("client_thread_start:malloc");
		return -1;
	}

	ct_arg->cfd = cfd;
	ct_arg->client_thread = client_thread;
	ct_arg->sigpipe = sigpipe;

	/* Start client thread. */
	err = pthread_create(
		&ct_arg->tid,
		NULL,
		client_thread_starter,
		ct_arg
	);

	if (err != 0) {
		print_error("client_thread_start:pthread_create", err);
		free(ct_arg);
		return -1;
	}

	return 0;
}

/**
 * Stops all client threads.
 */
void client_threads_stop()
{
	char sig = 1;
	pthread_t tid;
	int err;

	/* Signal client thread joiner to quit. */
	write(client_threads_pipe[1], &sig, 1);

	/* Join client thread joiner. */
	err = pthread_join(client_thread_joiner_id, NULL);

	if (err != 0) {
		print_error("client_threads_stop:pthread_join", err);
	}

	/* Wait for all remaining client threads to quit. */
	for (;;) {
		/* Obtain client threads mutex lock. */
		err = pthread_mutex_lock(&client_threads_lock);

		if (err != 0) {
			print_error(
				"client_threads_stop:pthread_mutex_lock",
				err
			);
			return;
		}

		if (client_threads_len == 0) {
			/* Release client threads mutex lock. */
			err = pthread_mutex_unlock(&client_threads_lock);

			if (err != 0) {
				print_error(
					"client_threads_stop:"
					"pthread_mutex_unlock",
					err
				);
			}

			break;
		}

		tid = client_threads[0].tid;

		/* Release client threads mutex lock. */
		err = pthread_mutex_unlock(&client_threads_lock);

		if (err != 0) {
			print_error(
				"client_threads_stop:pthread_mutex_unlock",
				err
			);
			return;
		}

		/* Wait for client thread to quit. */
		err = pthread_join(tid, NULL);

		if (err != 0) {
			print_error(
				"client_threads_stop:pthread_join",
				err
			);
			return;
		}

		/* Remove client thread from client threads array. */
		client_threads_remove(tid);
	}
}
