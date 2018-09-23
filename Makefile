CC = gcc
CFLAGS += -fPIC -fno-common -Wall -Wextra -Werror


.PHONY: all clean install test

all: libduckbloom.so

logc.o: logc/log.c logc/log.h
	$(CC) $(CFLAGS) -c -o $@ $<

mmapf.o: mmapf.c mmapf.h
	$(CC) $< $(CFLAGS) -c -o $@

duckbloom.o: duckbloom.c duckbloom.h
	$(CC) $< $(LOGINCLUDE) $(CFLAGS) -c -I.

libduckbloom.so: duckbloom.o logc.o mmapf.o
	$(CC) $^ $(CFLAGS) -shared -o $@

libduckbloomstatic.a: duckbloom.o logc.o mmapf.o
	ar rcs $@ $^

test:
	python3 duckbloom.py

clean:
	-rm -f *.o *.so *.a *.blm

install:
	cp libduckbloom.so /usr/local/lib/
	cp duckbloom.h /usr/local/include/

