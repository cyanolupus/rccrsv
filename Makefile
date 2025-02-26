APP_NAME=reccursive
CFLAGS=-std=c11 -g
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

reccursive: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

$(OBJS): reccursive.h

clean:
	rm -f reccursive *.o tmp/*

.PHONY: reccursive_test clean