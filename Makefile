APP_NAME=reccursive
CFLAGS=-std=c11 -g
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

reccursive: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

$(OBJS): reccursive.h

reccursive_test:
	@$(MAKE) assert LEFT_PARAM="1+1" RIGHT_PARAM=2
	@$(MAKE) assert LEFT_PARAM="1+2*3" RIGHT_PARAM=7
	@$(MAKE) assert LEFT_PARAM="1+2*3+4" RIGHT_PARAM=11
	@$(MAKE) assert LEFT_PARAM="1+2*3+4*5" RIGHT_PARAM=27
	@$(MAKE) assert LEFT_PARAM="5/2" RIGHT_PARAM=2
	@$(MAKE) assert LEFT_PARAM="5/2+1" RIGHT_PARAM=3
	@$(MAKE) assert LEFT_PARAM="1+(2*3)+4*5" RIGHT_PARAM=27
	@$(MAKE) assert LEFT_PARAM="(1+2)*3+4*5" RIGHT_PARAM=29
	@$(MAKE) assert LEFT_PARAM="1+2*(3+4)*5" RIGHT_PARAM=71
	@$(MAKE) assert LEFT_PARAM="-10+20" RIGHT_PARAM=10
	@$(MAKE) assert LEFT_PARAM="-(3+2)*6+44" RIGHT_PARAM=14
	@$(MAKE) assert LEFT_PARAM="(3+2)==5" RIGHT_PARAM=1
	@$(MAKE) assert LEFT_PARAM="(3+2)==6" RIGHT_PARAM=0
	@$(MAKE) assert LEFT_PARAM="(3+2)!=5" RIGHT_PARAM=0
	@$(MAKE) assert LEFT_PARAM="(3+2)!=6" RIGHT_PARAM=1
	@$(MAKE) assert LEFT_PARAM="(3+2)<5" RIGHT_PARAM=0
	@$(MAKE) assert LEFT_PARAM="(3+2)<6" RIGHT_PARAM=1
	@$(MAKE) assert LEFT_PARAM="(3+2)<=5" RIGHT_PARAM=1
	@$(MAKE) assert LEFT_PARAM="(3+2)<=4" RIGHT_PARAM=0
	@$(MAKE) assert LEFT_PARAM="(3+2)>5" RIGHT_PARAM=0
	@$(MAKE) assert LEFT_PARAM="(3+2)>6" RIGHT_PARAM=0
	@$(MAKE) assert LEFT_PARAM="(3+2)>=5" RIGHT_PARAM=1
	@$(MAKE) assert LEFT_PARAM="(3+2)>=6" RIGHT_PARAM=0

assert: reccursive
	@./reccursive "$(LEFT_PARAM)" > tmp.s
	@cc -o tmp tmp.s
	@bash assert.sh ./tmp "$(LEFT_PARAM)" "$(RIGHT_PARAM)"
	@rm -f tmp tmp.s

clean:
	rm -f reccursive

.PHONY: reccursive_test clean