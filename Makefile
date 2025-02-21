APP_NAME=reccursive
CFLAGS=-std=c11 -g

reccursive: reccursive.c

reccursive_test: reccursive
	./reccursive 123 > tmp.s
	cc -o tmp tmp.s
	./tmp || if [ $$? -eq 123 ]; then exit 0; else exit 1; fi

clean:
	rm -f reccursive tmp tmp.s

.PHONY: reccursive_test clean