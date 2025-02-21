APP_NAME=reccursive
CFLAGS=-std=c11 -g
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)
SEP = '$$'
TEST_LENGTH = $(shell cat testcase.json | jq '. | length')
SEQUENCE = $(shell awk 'BEGIN{for(i=0;i<$(TEST_LENGTH);i++)print i}')

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

$(foreach TEST,$(SEQUENCE), $(eval $(call TEST_RULE,$(shell cat testcase.json | jq -r '.[$(TEST)].name'),$(shell cat testcase.json | jq -r '.[$(TEST)].input'),$(shell cat testcase.json | jq -r '.[$(TEST)].output'))))

reccursive_test: $(foreach TEST,$(SEQUENCE),assert_$(shell cat testcase.json | jq -r '.[$(TEST)].name'))

clean:
	rm -f reccursive

.PHONY: reccursive_test clean $(foreach TEST,$(SEQUENCE),assert_$(shell cat testcase.json | jq -r '.[1].name'))