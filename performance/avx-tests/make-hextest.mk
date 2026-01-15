
# a simple makefile to compile hextest.c
#
# gcc -o hextest  /usr/local/lib/libhexsimd.so  hextest.c
# it requires libhexsimd.so to be in /usr/local/lib
# or you can set LD_LIBRARY_PATH to point to the directory
# where libhexsimd.so is located
#
# link the library to be static
#
# to run the hextest program, use:
# ./hextest

#static flags
#LDFLAGS = -L/usr/local/lib -l:libhexsimd.a

CC = gcc
CFLAGS = -O3 -Wall
#LDFLAGS = -L/usr/local/lib -lhexsimd
LDFLAGS = -L/usr/local/lib -l:libhexsimd.a
TARGET = hextest
SRCS = hextest.c
OBJS = $(SRCS:.c=.o)
.PHONY: all clean
all: $(TARGET)
$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -f $(TARGET) $(OBJS)	

