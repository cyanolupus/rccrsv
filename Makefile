APP_NAME=reccursive
CFLAGS=-std=c11 -g
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)
SEP = '$$'
TESTS = ADD$(SEP)"1+1;"$(SEP)2 \
	MUL$(SEP)"2*3;"$(SEP)6 \
	ADD_MUL1$(SEP)"1+2*3+4;"$(SEP)11 \
	ADD_MUL2$(SEP)"1+2*3+4*5;"$(SEP)27 \
	DIV$(SEP)"5/2;"$(SEP)2 \
	DIV_ADD$(SEP)"5/2+1;"$(SEP)3 \
	PRIORITIES0$(SEP)"1+(2*3)+4*5;"$(SEP)27 \
	PRIORITIES1$(SEP)"(1+2)*3+4*5;"$(SEP)29 \
	PRIORITIES2$(SEP)"1+2*(3+4)*5;"$(SEP)71 \
	UNARY$(SEP)"-10+20;"$(SEP)10 \
	UNARY_PRIORITIES$(SEP)"-(3+2)*6+44;"$(SEP)14 \
	EQ0$(SEP)"(3+2)==5;"$(SEP)1 \
	EQ1$(SEP)"(3+2)==6;"$(SEP)0 \
	NE0$(SEP)"(3+2)!=5;"$(SEP)0 \
	NE1$(SEP)"(3+2)!=6;"$(SEP)1 \
	LT0$(SEP)"(3+2)<5;"$(SEP)0 \
	LT1$(SEP)"(3+2)<6;"$(SEP)1 \
	LE0$(SEP)"(3+2)<=5;"$(SEP)1 \
	LE1$(SEP)"(3+2)<=4;"$(SEP)0 \
	GT0$(SEP)"(3+2)>5;"$(SEP)0 \
	GT1$(SEP)"(3+2)>6;"$(SEP)0 \
	GE0$(SEP)"(3+2)>=5;"$(SEP)1 \
	GE1$(SEP)"(3+2)>=6;"$(SEP)0 \
	ASSIGN0$(SEP)"a=1;(a+2)*3+4*5;"$(SEP)29 \
	ASSIGN1$(SEP)"a=1;b=2;(a+b)*3+4*5;"$(SEP)29 \
	ASSIGN2$(SEP)"a=(1*2);(a+2)*3+4*5;"$(SEP)32 \
	ASSIGN3$(SEP)"a=(1*2);n=(a+2)*3+4*5;m=(n+a);m;"$(SEP)34

reccursive: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

$(OBJS): reccursive.h

define TEST_RULE
assert_$(1): reccursive
	@mkdir -p tmp
	@./reccursive "$(2)" > tmp/assert_$(1).s
	@cc -o tmp/assert_$(1) tmp/assert_$(1).s
	@bash assert.sh ./tmp/assert_$(1) "$(2)" "$(3)" $(VERBOSE)
	@rm -f tmp/assert_$(1) tmp/assert_$(1).s
endef

$(foreach TEST,$(TESTS),$(eval $(call TEST_RULE,$(shell echo $(TEST) | cut -d$$ -f1),$(shell echo $(TEST) | cut -d$$ -f2),$(shell echo $(TEST) | cut -d$$ -f3))))

reccursive_test: $(foreach TEST,$(TESTS),assert_$(shell echo $(TEST) | cut -d$$ -f1))

clean:
	rm -f reccursive

.PHONY: reccursive_test clean $(foreach TEST,$(TESTS),assert_$(shell echo $(TEST) | cut -d$$ -f1))