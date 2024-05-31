CC = gcc
CFLAGS = -pthread -Wall -D_GNU_SOURCE
LDFLAGS = -lm

all: lab5_int

%: %.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f lab5_double
