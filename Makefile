CC = gcc
BIN = ./bin
DIRS = cases
CFLAGS = -g -ggdb
GST_PKG = $(shell pkg-config --cflags --libs gstreamer-1.0)

CASES = basic1 basic2 basic2filter basic3
build_cases:
	for i in $(CASES); do \
		$(CC) cases/$$i.c -o $(BIN)/case_$$i $(GST_PKG) $(CFLAGS) || exit 1; \
	done

clean:
	rm -f bin/*

all: build_cases
