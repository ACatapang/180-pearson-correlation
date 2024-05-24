CC = gcc
CFLAGS = -pthread -Wall -lrt -D_GNU_SOURCE
LDFLAGS = -lm

FILENAME ?= lab05_catapang

all: $(FILENAME)

$(FILENAME): $(FILENAME).c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f $(FILENAME)
