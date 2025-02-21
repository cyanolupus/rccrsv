APP_NAME=reccursive
CFLAGS=-std=c11 -g

reccursive: reccursive.c

reccursive_test: reccursive
	./reccursive " 1 + 2-3+4 " > tmp.s
	cc -o tmp tmp.s
	./tmp || if [ $$? -eq 4 ]; then exit 0; else exit 1; fi

clean:
	rm -f reccursive tmp tmp.s

.PHONY: reccursive_test clean