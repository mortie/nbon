#include "test.h"

#include <algorithm>
#include <vector>
#include <iostream>
#include <cstring>

static std::vector<TestCase *> testCases;

TestCase::TestCase(
		const char *file, int line, const char *name, void (*func)()):
		file(file), line(line), name(name), func(func) {
	testCases.push_back(this);
}

static bool currentTestFailed = false;

static void breakpoint() {
}

void onCheckFailure(const char *file, int line, const char *msg) {
	std::cout << "    Check error: " << file << ":" << line << ": " << msg << '\n';
	currentTestFailed = true;
	breakpoint();
}

int main() {
	std::sort(testCases.begin(), testCases.end(), [](auto a, auto b) {
		int cmp = std::strcmp(a->file, b->file);
		if (cmp < 0) {
			return true;
		} else if (cmp > 0) {
			return false;
		} else if (a->line < b->line) {
			return true;
		} else {
			return false;
		}
	});

	int tests = 0;
	int successes = 0;

	const char *prevFile = nullptr;
	for (auto testCase: testCases) {
		if (testCase->file != prevFile) {
			std::cout << '\n';
			std::cout << "Running " << testCase->file << "...\n";
			prevFile = testCase->file;
		}

		std::cout << "  " << testCase->name << '\n';

		tests += 1;
		try {
			currentTestFailed = false;
			testCase->func();
			if (!currentTestFailed) {
				successes += 1;
			}
		} catch (std::exception &ex) {
			std::cout << "    Exception: " << ex.what() << '\n';
			breakpoint();
		}
	}

	std::cout << '\n';
	std::cout << successes << "/" << tests << " tests succeeded.\n";
	return tests == successes ? 0 : 1;
}
