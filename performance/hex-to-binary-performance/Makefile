CC = gcc
CFLAGS = -Wall -g -mavx2 -msse2 -mssse3 -O3 -O2
#CFLAGS =  -g -mavx2 -msse2 -mssse3 -O3 -O2

# Define your executables (targets)
TARGETS = hex-to-bin-tests-new hex-to-bin-to-hex-tests

# Default target: builds all other targets
all: $(TARGETS)

# Rule for building program1
hex-to-bin-to-hex-tests: hex-to-bin-to-hex-tests.o
	$(CC) $(CFLAGS) -o $@ $^

# Rule for building program2
hex-to-bin-tests-new: hex-to-bin-tests-new.o
	$(CC) $(CFLAGS) -o $@ $^

# Generic rule for compiling .c files into .o files
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Phony target for cleaning up generated files
.PHONY: clean
clean:
	rm -f $(TARGETS) *.o


