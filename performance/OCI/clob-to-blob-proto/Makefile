# Makefile for clob_to_blob using OCILib

CC=gcc
CFLAGS=-Wall -O3 -std=c99 -msse2 -g
LDFLAGS=-l:libocilib.a -lclntsh -L/usr/local/lib -L/u01/app/oracle/product/19c/lib

TARGET=clob-to-blob-aq

TARGET2=clob-to-blob-aq-MODS

all: $(TARGET) $(TARGET2)

$(TARGET): clob-to-blob-with-aq.c
	$(CC) $(CFLAGS) -o $(TARGET) clob-to-blob-with-aq.c $(LDFLAGS)

$(TARGET2): clob-to-blob-with-aq-MODS.c
	$(CC) $(CFLAGS) -o $(TARGET2) clob-to-blob-with-aq-MODS.c $(LDFLAGS)

clean:
	rm -f $(TARGET) $(TARGET2)
