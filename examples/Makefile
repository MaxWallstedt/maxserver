# Copyright © 2018  Max Wällstedt <max.wallstedt@gmail.com>
#
# This file is part of maxserver.
#
# maxserver is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# maxserver is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public
# License along with maxserver.  If not, see
# <http://www.gnu.org/licenses/>.

RM = rm -f
CC = gcc
CFLAGS = -g -pedantic -Wall -Wextra -Werror
LDFLAGS = -lmaxserver -pthread

all: echo_client echo_server

echo_client: echo_client.o client_socket.o
	@echo -e "LD\t$@"
	@$(CC) $(CFLAGS) -o $@ $^

echo_client.o: echo_client.c client_socket.h
	@echo -e "CC\t$@"
	@$(CC) -c $(CFLAGS) -o $@ $<

client_socket.o: client_socket.c client_socket.h
	@echo -e "CC\t$@"
	@$(CC) -c $(CFLAGS) -o $@ $<

echo_server: echo_server.o
	@echo -e "LD\t$@"
	@$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

echo_server.o: echo_server.c
	@echo -e "CC\t$@"
	@$(CC) -c $(CFLAGS) -o $@ $<

.PHONY: clean

clean:
	@echo -e "RM\techo_client"
	@$(RM) echo_client
	@echo -e "RM\techo_client.o"
	@$(RM) echo_client.o
	@echo -e "RM\tclient_socket.o"
	@$(RM) client_socket.o
	@echo -e "RM\techo_server"
	@$(RM) echo_server
	@echo -e "RM\techo_server.o"
	@$(RM) echo_server.o
