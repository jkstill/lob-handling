# Makefile for clob_to_blob using OCILib

CC=gcc
CFLAGS=-Wall -O3 -std=c99 -msse2
LDFLAGS=-l:libocilib.a -lclntsh -L/usr/local/lib -L/u01/app/oracle/product/19c/lib

TARGET=clob-to-blob-aq

all: $(TARGET)

$(TARGET): clob-to-blob-with-aq.c
	$(CC) $(CFLAGS) -o $(TARGET) clob-to-blob-with-aq.c $(LDFLAGS)

clean:
	rm -f $(TARGET)
