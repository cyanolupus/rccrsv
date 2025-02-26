APP_NAME=rccrsv
CFLAGS=-std=c11 -g
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

rccrsv: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

$(OBJS): rccrsv.h

clean:
	rm -f rccrsv *.o tmp/*

.PHONY: rccrsv_test clean