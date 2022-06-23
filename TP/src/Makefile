SOURCES=$(wildcard *.c) $(wildcard **/*.c)
OBJECTS=$(SOURCES:.c=.o)

CC = gcc
CFLAGS = -std=gnu99
CFLAGS_PROD = -w -O2
CFLAGS_TEST = -g -Wall -Wshadow
CFLAGS_DEBUG = -g -Wall -Wshadow -fsanitize=leak -fsanitize=address -fsanitize=undefined -DDEBUG=1

env ?= prod

ifeq ($(env), test)
	CFLAGS += $(CFLAGS_TEST)
endif
ifeq ($(env), debug)
	CFLAGS += $(CFLAGS_DEBUG)
endif
ifeq ($(env), prod)
	CFLAGS += $(CFLAGS_PROD)
endif

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

all: $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o main

run:
	./main

clean:
	rm -f *.o main **/*.o

.PHONY: all run clean