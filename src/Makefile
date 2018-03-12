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
TARGET = maxserver

all: lib$(TARGET).so.1.0

lib$(TARGET).so.1.0: \
	maxserver.o \
	print_error.o \
	server_socket.o \
	accept_thread.o \
	client_thread.o
	@echo -e "LD\t$@"
	@$(CC) -shared -Wl,-soname,lib$(TARGET).so.1 -o $@ $^

maxserver.o: \
	maxserver.c \
	maxserver.h \
	print_error.h \
	server_socket.h \
	accept_thread.h
	@echo -e "CC\t$<"
	@$(CC) -c $(CFLAGS) -fPIC -o $@ $<

print_error.o: \
	print_error.c \
	print_error.h
	@echo -e "CC\t$<"
	@$(CC) -c $(CFLAGS) -fPIC -o $@ $<

server_socket.o: \
	server_socket.c \
	server_socket.h \
	print_error.h
	@echo -e "CC\t$<"
	@$(CC) -c $(CFLAGS) -fPIC -o $@ $<

accept_thread.o: \
	accept_thread.c \
	accept_thread.h \
	print_error.h \
	client_thread.h
	@echo -e "CC\t$<"
	@$(CC) -c $(CFLAGS) -fPIC -o $@ $<

client_thread.o: \
	client_thread.c \
	client_thread.h \
	print_error.h
	@echo -e "CC\t$<"
	@$(CC) -c $(CFLAGS) -fPIC -o $@ $<

.PHONY: clean

clean:
	@echo -e "RM\tlib$(TARGET).so.1.0"
	@$(RM) lib$(TARGET).so.1.0
	@echo -e "RM\tmaxserver.o"
	@$(RM) maxserver.o
	@echo -e "RM\tprint_error.o"
	@$(RM) print_error.o
	@echo -e "RM\tserver_socket.o"
	@$(RM) server_socket.o
	@echo -e "RM\taccept_thread.o"
	@$(RM) accept_thread.o
	@echo -e "RM\tclient_thread.o"
	@$(RM) client_thread.o
