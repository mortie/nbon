SANITIZE ?= address,undefined
CMD ?=

CFLAGS += -std=c++20 -g -Wall -Wextra -Wpedantic -Iinclude

ifneq ($(SANITIZE),)
CFLAGS += -fsanitize=$(SANITIZE)
endif

HDRS = tests/test.h include/sbon.h
SRCS = tests/main.cc tests/read.cc tests/write.cc

run-tests: $(HDRS) $(SRCS)
	$(CXX) -o $@ $(CFLAGS) $(SRCS)

.PHONY: check
check: run-tests
	$(CMD) ./run-tests

.PHONY: clean
clean:
	rm -f run-tests
