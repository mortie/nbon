#include <sbon.h>

#include <limits>
#include <sstream>

#include "test.h"

static char hexDigit(unsigned char nibble) {
	if (nibble <= 9) {
		return '0' + nibble;
	} else if (nibble <= 15) {
		return 'a' + (nibble - 10);
	} else {
		throw std::runtime_error("Invalid hex nibble");
	}
}

static void checkEq(std::string actual, const char *expected) {
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
		std::cout << "    Expected: " << expected << '\n';
		std::cout << "      Actual: " << expanded << '\n';
	}

	CHECK_EQ(expanded, std::string_view(expected));
}

TEST_CASE("Basic") {
	std::stringstream ss;
	sbon::Writer w(&ss);

	w.writeTrue();
	w.writeFalse();
	w.writeNull();
	w.writeBool(false);
	w.writeBool(true);

	checkEq(ss.str(), "TFNFT");
}

TEST_CASE("Strings") {
	std::stringstream ss;
	sbon::Writer w(&ss);

	w.writeString("Hello World!");
	checkEq(ss.str(), "SHello World!<00>");
}

TEST_CASE("Binary") {
	std::stringstream ss;
	sbon::Writer w(&ss);

	w.writeBinary("Hello", 5);
	checkEq(ss.str(), "B<05>Hello");
}

TEST_CASE("Single byte integers") {
	std::stringstream ss;
	sbon::Writer w(&ss);

	// 0-9
	w.writeInt(0);
	w.writeUInt(0);
	w.writeInt(1);
	w.writeUInt(1);
	w.writeInt(2);
	w.writeUInt(2);
	w.writeInt(8);
	w.writeUInt(8);
	w.writeInt(9);
	w.writeUInt(9);

	// Positive, single-byte
	w.writeInt(10);
	w.writeUInt(10);
	w.writeInt(33);
	w.writeUInt(33);
	w.writeInt(127);
	w.writeUInt(127);

	// Negative, single-byte
	w.writeInt(-1);
	w.writeInt(-66);
	w.writeInt(-127);

	checkEq(ss.str(),
		"0011228899"
		"+<0a>+<0a>+<21>+<21>+<7f>+<7f>"
		"-<01>-<42>-<7f>");
}

TEST_CASE("Multi byte integers") {
	std::stringstream ss;
	sbon::Writer w(&ss);

	w.writeInt(128);
	w.writeUInt(128);

	w.writeUInt(0xffffffffull);

	w.writeInt(-0x7fffffffffffffffll);
	w.writeUInt(0xffffffffffffffffll);

	checkEq(ss.str(),
		"+<80><01>"
		"+<80><01>"
		"+<ff><ff><ff><ff><0f>"
		"-<ff><ff><ff><ff><ff><ff><ff><ff><7f>"
		"+<ff><ff><ff><ff><ff><ff><ff><ff><ff><01>");
}

TEST_CASE("Floats") {
	std::stringstream ss;
	sbon::Writer w(&ss);

	w.writeFloat(10);
	w.writeFloat(10040.33);
	w.writeFloat(0.1);
	w.writeFloat(-11);
	w.writeFloat(std::numeric_limits<float>::infinity());
	checkEq(ss.str(),
		"f<00><00><20><41>"
		"f<52><e1><1c><46>"
		"f<cd><cc><cc><3d>"
		"f<00><00><30><c1>"
		"f<00><00><80><7f>");
}

TEST_CASE("Doubles") {
	std::stringstream ss;
	sbon::Writer w(&ss);

	w.writeDouble(10);
	w.writeDouble(10040.33);
	w.writeDouble(0.1);
	w.writeDouble(-11);
	w.writeDouble(std::numeric_limits<double>::infinity());
	checkEq(ss.str(),
		"d<00><00><00><00><00><00><24><40>"
		"d<d7><a3><70><3d><2a><9c><c3><40>"
		"d<9a><99><99><99><99><99><b9><3f>"
		"d<00><00><00><00><00><00><26><c0>"
		"d<00><00><00><00><00><00><f0><7f>");
}

TEST_CASE("Arrays") {
	std::stringstream ss;
	sbon::Writer w(&ss);

	w.writeArray([](sbon::Writer w) {
		w.writeTrue();
		w.writeFalse();
		w.writeArray([](sbon::Writer w) {
			w.writeBool(false);
			w.writeBool(true);
		});
		w.writeNull();
	});

	checkEq(ss.str(), "[TF[FT]N]");
}

TEST_CASE("Objects") {
	std::stringstream ss;
	sbon::Writer w(&ss);

	w.writeObject([](sbon::ObjectWriter w) {
		w.key("Hello").writeTrue();
		w.key("Goodbye").writeFalse();
		w.key("SubObj").writeObject([](sbon::ObjectWriter w) {
			w.key("hello world").writeInt(3);
		});
		w.key("x").writeInt(2);
		w.key("y").writeInt(4);
	});

	checkEq(ss.str(),
		"{Hello<00>TGoodbye<00>F"
		"SubObj<00>{hello world<00>3}"
		"x<00>2y<00>4}");
}

TEST_CASE("Logic error checking") {
	std::stringstream ss;
	sbon::Writer w(&ss);

	bool threw = false;
	try {
		w.writeArray([&](auto) {
			// Write to toplevel
			w.writeInt(10);
		});
	} catch (sbon::LogicError &err) {
		threw = true;
	}
	CHECK(threw);

	threw = false;
	try {
		w.writeObject([&](auto) {
			// Write to toplevel
			w.writeInt(10);
		});
	} catch (sbon::LogicError &err) {
		threw = true;
	}
	CHECK(threw);
}
