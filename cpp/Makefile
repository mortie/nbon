SANITIZE ?= address,undefined
CMD ?=

CFLAGS += -std=c++20 -g -Wall -Wextra -Wpedantic -Iinclude

ifneq ($(SANITIZE),)
CFLAGS += -fsanitize=$(SANITIZE)
endif

run-tests: tests/main.cc tests/write.cc tests/util.h include/nbon.h
	$(CXX) -o $@ $(CFLAGS) tests/main.cc tests/write.cc

.PHONY: check
check: run-tests
	$(CMD) ./run-tests

.PHONY: clean
clean:
	rm -f run-tests
