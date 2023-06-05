LIBS = xcb xcb-xfixes x11 x11-xcb gl

CC_FLAGS = -I ../include -c -fsanitize=address -g `pkg-config --cflags $(LIBS)` -Wno-implicit-function-declaration
LD_FLAGS = -fsanitize=address -g `pkg-config --libs $(LIBS)` -lm

CC ?= gcc
LD = $(CC)


SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)



%.o: %.c
	$(CC) $(CC_FLAGS) $< -o $@

all: $(OBJECTS)
	$(LD) $(LD_FLAGS) $^ -o ../bin/voxelizer


clean:
	rm *.o -rf
