LIBS = libpng

CC = gcc -I ./include -c -fsanitize=address -g `pkg-config --cflags $(LIBS)`
LD = gcc -fsanitize=address -g `pkg-config --libs $(LIBS)` -lm


SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)



%.o: %.c
	$(CC) $< -o $@

all: $(OBJECTS)
	$(LD) $^ -o main


clean:
	rm *.o main -rf
