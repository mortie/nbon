SANITIZE ?= address,undefined
CMD ?=

CFLAGS += -std=c++20 -g -Wall -Wextra -Wpedantic -Iinclude

ifneq ($(SANITIZE),)
CFLAGS += -fsanitize=$(SANITIZE)
endif


.PHONY: all
all: sbon-to-json

TEST_HDRS = tests/test.h include/sbon.h
TEST_SRCS = tests/main.cc tests/cases/read.cc tests/cases/write.cc
test-sbon: $(TEST_HDRS) $(TEST_SRCS)
	$(CXX) -o $@ $(CFLAGS) $(TEST_SRCS) -Itests

sbon-to-json: examples/sbon-to-json.cc include/sbon.h
	$(CXX) -o $@ $(CFLAGS) -O2 $<

.PHONY: check
check: test-sbon
	$(CMD) ./test-sbon

.PHONY: clean
clean:
	rm -f test-sbon sbon-to-json
