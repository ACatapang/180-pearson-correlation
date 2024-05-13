CC = gcc
CFLAGS = -lm -pthread -Wall -lrt -D_GNU_SOURCE

FILENAME := $(FILENAME)

all: $(FILENAME)

$(FILENAME): $(FILENAME).c
	$(CC) $(FILENAME).c -o $(FILENAME) $(CFLAGS)
    
clean:
	rm -f $(FILENAME)
