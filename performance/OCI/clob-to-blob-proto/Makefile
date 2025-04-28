# Makefile for clob_to_blob using OCILib

CC=gcc
#CFLAGS=-Wall -O3 -std=c99 -msse2 -g -fsanitize=address
CFLAGS=-Wall -O3 -std=c99 -msse2 -g
LDFLAGS=-l:libocilib.a -lclntsh -L/usr/local/lib -L/u01/app/oracle/product/19c/lib

LSAN_FILE := lsan.supp

# make clob-to-blob-with-aq
c2b := clob-to-blob-aq
# make clob-to-blob-with-aq-MODS
c2b-mods := clob-to-blob-aq-MODS
# make clob-to-blob-sql-gen
rebind-test := rebind-test
# make clob-to-blob-sql-gen
c2b-sql-gen := clob-to-blob-sql-gen

all: $(c2b) $(c2b-mods) $(rebind-test) $(c2b-sql-gen)

$(c2b): clob-to-blob-with-aq.c
	$(CC) $(CFLAGS) -o $(c2b) clob-to-blob-with-aq.c $(LDFLAGS)

$(c2b-mods): clob-to-blob-with-aq-MODS.c
	$(CC) $(CFLAGS) -o $(c2b-mods) clob-to-blob-with-aq-MODS.c $(LDFLAGS)

$(c2b-sql-gen): clob-to-blob-sql-gen.c
	$(CC) $(CFLAGS) -o $(c2b-sql-gen) clob-to-blob-sql-gen.c $(LDFLAGS)

$(rebind-test): rebind-test.c
	$(CC) $(CFLAGS) -o $(rebind-test) rebind-test.c $(LDFLAGS)

clean-rebind-test:
	rm -f $(rebind-test)

clean-sql-gen:
	rm -f $(c2b-sql-gen)

clean-c2b:
	rm -f $(c2b) $(c2b-mods)

clean:
	rm -f $(c2b) $(c2b-mods) $(rebind-test) $(c2b-sql-gen)

# create lsan suppressions file for Leak Sanitizer
lsan:
	@echo "# suppressions for libclntsh.so" > $(LSAN_FILE)
	@echo "leak:libclntsh.so" >> $(LSAN_FILE)

run:
	# create lsan suppressions file if not exists
	if [ ! -f $(LSAN_FILE) ]; then \
		$(MAKE) lsan; \
	fi
	# run the program with lsan
	#LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libasan.so.5 \
	LSAN_OPTIONS=suppressions=$(LSAN_FILE) ./$(c2b) 
