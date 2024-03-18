CC = gcc
CFLAGS = -lm -pthread
FILENAME = lab02_catapang

all: $(FILENAME)

$(FILENAME): $(FILENAME).c
	$(CC) -Wall -pthread $(FILENAME).c -o $(FILENAME) $(CFLAGS) -lm
    
clean:
	rm -f $(FILENAME)
