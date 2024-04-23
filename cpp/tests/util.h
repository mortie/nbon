#include <stdexcept>
#include <iostream>
#include <doctest/doctest.h>

inline char hexDigit(unsigned char nibble) {
	if (nibble <= 9) {
		return '0' + nibble;
	} else if (nibble <= 15) {
		return 'a' + (nibble - 10);
	} else {
		throw std::runtime_error("Invalid hex nibble");
	}
}

inline void checkEq(std::string actual, const char *expected) {
	std::string expanded;

	const char *expCh = expected;
	for (char ch: actual) {
		if (expCh[0] == '<') {
			REQUIRE(expCh[1] != '\0');
			REQUIRE(expCh[2] != '\0');
			REQUIRE(expCh[3] == '>');

			unsigned char byte = (unsigned char)ch;
			expanded += '<';
			expanded += hexDigit(byte >> 4);
			expanded += hexDigit(byte & 0x0f);
			expanded += '>';
			expCh += 4;
		} else {
			expanded += ch;
			expCh += 1;
		}
	}

	// It's useful to have these on their own lines...
	if (expanded != expected) {
		std::cout << "Expected: " << expected << '\n';
		std::cout << "  Actual: " << expanded << '\n';
	}

	CHECK_EQ(expanded, std::string_view(expected));
}
