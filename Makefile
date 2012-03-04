# Copyright (c) 2012, Jan Vaughan
# All rights reserved.
#
# Makefile set up for Mac OS X. Changes will be required to support other
# systems. Assumes hid.c from hidapi has been copied into the powerup directory.

CC=gcc
CFLAGS=-Wall
OBJS=powerup.o powerlog6s.o hidselect.o hid.o flags.o
LIBS=-framework IOKit -framework CoreFoundation

all: powerup

powerup: $(OBJS)
	gcc $^ $(LIBS) -o $@

clean:
	rm -f *.o powerup
