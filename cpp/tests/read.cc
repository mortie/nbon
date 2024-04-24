#include <nbon.h>

#include <sstream>
#include <string_view>

#include "test.h"

TEST_CASE("Basic") {
	std::stringstream ss{"TFNFT"};
	nbon::Reader r(&ss);

	CHECK(r.getBool() == true);
	CHECK(r.getBool() == false);
	CHECK(r.getType() == nbon::Type::NIL);
	r.getNil();
	CHECK(r.getBool() == false);
	CHECK(r.getBool() == true);
	CHECK(!r.hasNext());
}

TEST_CASE("Strings") {
	char buf[] = "SHello World!\0";
	std::stringstream ss{std::string(buf, sizeof(buf) - 1)};
	nbon::Reader r(&ss);

	CHECK(r.getString() == "Hello World!");
	CHECK(!r.hasNext());
}

TEST_CASE("Binary") {
	char buf[] = "B\x0cHello World!";
	std::stringstream ss{std::string(buf, sizeof(buf) - 1)};
	nbon::Reader r(&ss);

	auto vec = r.getBinary();
	CHECK(vec.size() == 12);
	CHECK(std::string_view((char *)vec.data(), vec.size()) == "Hello World!");
	CHECK(!r.hasNext());
}

TEST_CASE("Single byte integers") {
	char buf[] =
		"3"
		"+\x0c"
		"+\x7f"
		"-\x02"
		"-\x7f";
	std::stringstream ss{std::string(buf, sizeof(buf) - 1)};
	nbon::Reader r(&ss);

	CHECK(r.getInt() == 3);
	CHECK(r.getInt() == 0x0c);
	CHECK(r.getInt() == 0x7f);
	CHECK(r.getInt() == -2);
	CHECK(r.getInt() == -0x7f);
	CHECK(!r.hasNext());

	char buf2[] =
		"3"
		"+\x0c"
		"+\x7f";
	ss = std::stringstream{std::string(buf2, sizeof(buf2) - 1)};
	r = nbon::Reader(&ss);
	CHECK(r.getUInt() == 3);
	CHECK(r.getUInt() == 0x0c);
	CHECK(r.getUInt() == 0x7f);
	CHECK(!r.hasNext());
}

TEST_CASE("Multi byte integers") {
	char buf[] =
		"+\x80\01"
		"+\x80\01"
		"+\xff\xff\xff\xff\x0f"
		"-\xff\xff\xff\xff\xff\xff\xff\xff\x7f"
		"+\xff\xff\xff\xff\xff\xff\xff\xff\xff\x01";
	std::stringstream ss{std::string(buf, sizeof(buf) - 1)};
	nbon::Reader r(&ss);

	CHECK(r.getInt() == 128);
	CHECK(r.getUInt() == 128);
	CHECK(r.getUInt() == 0xffffffffull);
	CHECK(r.getInt() == -0x7fffffffffffffffll);
	CHECK(r.getUInt() == 0xffffffffffffffffll);
	CHECK(!r.hasNext());
}

TEST_CASE("Floats") {
	char buf[] =
		"f\x00\x00\x20\x41"
		"f\x52\xe1\x1c\x46"
		"f\xcd\xcc\xcc\x3d"
		"f\x00\x00\x30\xc1"
		"f\x00\x00\x80\x7f";
	std::stringstream ss{std::string(buf, sizeof(buf) - 1)};
	nbon::Reader r(&ss);

	CHECK(r.getFloat() == 10.0f);
	CHECK(r.getFloat() == 10040.33f);
	CHECK(r.getFloat() == 0.1f);
	CHECK(r.getFloat() == -11.0f);
	CHECK(r.getFloat() == std::numeric_limits<float>::infinity());
	CHECK(!r.hasNext());
}

TEST_CASE("Doubles") {
	char buf[] =
		"d\x00\x00\x00\x00\x00\x00\x24\x40"
		"d\xd7\xa3\x70\x3d\x2a\x9c\xc3\x40"
		"d\x9a\x99\x99\x99\x99\x99\xb9\x3f"
		"d\x00\x00\x00\x00\x00\x00\x26\xc0"
		"d\x00\x00\x00\x00\x00\x00\xf0\x7f";
	std::stringstream ss{std::string(buf, sizeof(buf) - 1)};
	nbon::Reader r(&ss);

	CHECK(r.getDouble() == 10);
	CHECK(r.getDouble() == 10040.33);
	CHECK(r.getDouble() == 0.1);
	CHECK(r.getDouble() == -11);
	CHECK(r.getDouble() == std::numeric_limits<double>::infinity());
	CHECK(!r.hasNext());
}

TEST_CASE("Arrays") {
	std::stringstream ss{"[T[FF]3]"};
	nbon::Reader r(&ss);

	r.getArray([](nbon::ArrayReader arr) {
		CHECK(arr.next().getBool() == true);
		arr.next().getArray([](nbon::ArrayReader arr) {
			CHECK(arr.next().getBool() == false);
			CHECK(arr.next().getBool() == false);
			CHECK(!arr.hasNext());
		});
		CHECK(arr.next().getUInt() == 3);
	});

	CHECK(!r.hasNext());
}

TEST_CASE("Objects") {
	char buf[] = "{Hello\0FSub\0{x\0Ny\0N}last\0T}";
	std::stringstream ss{std::string(buf, sizeof(buf) - 1)};
	nbon::Reader r(&ss);

	r.getObject([](nbon::ObjectReader obj) {
		std::string key;
		nbon::Reader val;
		val = obj.next(key);
		CHECK(key == "Hello");
		CHECK(val.getBool() == false);

		val = obj.next(key);
		CHECK(key == "Sub");
		val.getObject([&](nbon::ObjectReader obj) {
			val = obj.next(key);
			CHECK(key == "x");
			CHECK(val.getType() == nbon::Type::NIL);
			val.getNil();

			val = obj.next(key);
			CHECK(key == "y");
			CHECK(val.getType() == nbon::Type::NIL);
			val.getNil();
		});

		val = obj.next(key);
		CHECK(key == "last");
		CHECK(val.getBool() == true);
	});

	CHECK(!r.hasNext());
}
