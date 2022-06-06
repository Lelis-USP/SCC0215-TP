SOURCES=$(wildcard *.c) $(wildcard **/*.c)
OBJECTS=$(SOURCES:.c=.o)

CC = gcc
CFLAGS = -std=gnu99
CFLAGS_PROD = -w -O2
CFLAGS_TEST = -g -Wall -Wshadow -fsanitize=address -fsanitize=undefined

env ?= prod

ifeq ($(env), test)
	CFLAGS += $(CFLAGS_TEST)
else
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