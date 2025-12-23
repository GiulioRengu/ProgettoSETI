CC=gcc
CFLAGS=-Wall
DEPS=fun.h
EXEC=test

all: $(EXEC)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $<

test: test.o fun.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf $(EXEC) *.o