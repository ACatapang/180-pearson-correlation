CC = gcc
CFLAGS = -lm -pthread -Wall -lrt -D_GNU_SOURCE
FILENAME = lab02_catapang

all: $(FILENAME)

$(FILENAME): $(FILENAME).c
	$(CC)  $(FILENAME).c -o $(FILENAME) $(CFLAGS)
    
clean:
	rm -f $(FILENAME)
