APP_NAME=reccursive
CFLAGS=-std=c11 -g

reccursive: reccursive.c

reccursive_test:
	@$(MAKE) assert LEFT_PARAM="1+1" RIGHT_PARAM=2
	@$(MAKE) assert LEFT_PARAM="1+2*3" RIGHT_PARAM=7
	@$(MAKE) assert LEFT_PARAM="1+2*3+4" RIGHT_PARAM=11
	@$(MAKE) assert LEFT_PARAM="1+2*3+4*5" RIGHT_PARAM=27
	@$(MAKE) assert LEFT_PARAM="1+(2*3)+4*5" RIGHT_PARAM=27
	@$(MAKE) assert LEFT_PARAM="(1+2)*3+4*5" RIGHT_PARAM=29
	@$(MAKE) assert LEFT_PARAM="1+2*(3+4)*5" RIGHT_PARAM=71

assert: reccursive
	./reccursive "$(LEFT_PARAM)" > tmp.s
	cc -o tmp tmp.s
	bash assert.sh ./tmp "$(LEFT_PARAM)" "$(RIGHT_PARAM)"
	@rm -f tmp tmp.s

clean:
	rm -f reccursive

.PHONY: reccursive_test clean