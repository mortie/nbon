#pragma once

#include <exception>
#include <string>

class TestFailure: public std::exception {
public:
	TestFailure(const char *file, int line, const char *msg) {
		str_ = file;
		str_ += ':';
		str_ += line;
		str_ += ": ";
		str_ += msg;
	}

	const char *what() const noexcept {
		return str_.c_str();
	}

private:
	std::string str_;
};

struct TestCase {
	TestCase(const char *file, int line, const char *name, void (*func)());

	const char *file;
	int line;
	const char *name;
	void (*func)() = nullptr;
};

void onCheckFailure(const char *file, int line, const char *msg);

#define CHECK(expr) do { \
	if (!(expr)) { \
		onCheckFailure(__FILE__, __LINE__, "(" #expr ")"); \
	} \
} while (0)

#define CHECK_EQ(a, b) CHECK((a) == (b))
#define CHECK_NEQ(a, b) CHECK((a) != (b))

#define REQUIRE(expr) do { \
	if (!(expr)) { \
		throw TestFailure(__FILE__, __LINE__, "Assertion failure: (" #expr ")"); \
	} \
} while (0)

#define COMBINE1(x,y) x##y
#define COMBINE(x,y) COMBINE1(x,y)
#define TEST_CASE(name) \
	static void COMBINE(testfunc_,__LINE__)(); \
	static TestCase COMBINE(testcase_,__LINE__) = { \
		__FILE__, __LINE__, name, COMBINE(testfunc_,__LINE__), \
	}; \
	static void COMBINE(testfunc_,__LINE__)()
