APP_NAME=rccrsv
CFLAGS=-std=c11 -g
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

rccrsv: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

$(OBJS): rccrsv.h

rccrsv_test: rccrsv selftest/testcases.c selftest/utils.c
	./rccrsv -o tmp/testcases.s selftest/testcases.c
	cc -c -o tmp/utils.o selftest/utils.c $(CFLAGS)
	cc -c -o tmp/testcases.o tmp/testcases.s $(CFLAGS)
	cc -o rccrsv_test tmp/testcases.o tmp/utils.o $(CFLAGS)
	./rccrsv_test

clean:
	rm -f rccrsv *.o tmp/*

.PHONY: clean rccrsv_test