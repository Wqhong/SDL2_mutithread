CC=gcc
CFLAGS=-g -std=c11 -Wall -Werror
TARGET=program
.PHONY: clean
all: $(TARGET)

nbody: nbody.c
	$(CC) $(CFLAGS) $^ -o $@ -lpthread

nbody-gui: nbodygui.c
	$(CC) $(CFLAGS) $^ -o $@ -lpthread -lSDL2 -lSDL2_gfx

test: nbodytest.c
	$(CC) $(CFLAGS) $^ -o $@ -lpthread -lcmocka

clean:
	rm -f *.o
	rm -f nbody-gui
	rm -f nbody
