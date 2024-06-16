#include <sbon.h>
#include <iostream>
#include <fstream>
#include <string_view>

static char hexNibble(unsigned char ch) {
	if (ch < 9) {
		return '0' + ch;
	} else {
		return 'A' + (ch - 10);
	}
}

static void writeString(std::string_view sv, std::ostream &os) {
	os << '"';
	for (char ch: sv) {
		if (ch == '"') {
			os << "\\\"";
		} else if (ch == '\\') {
			os << "\\\\";
		} else if (ch == '\b') {
			os << "\\b";
		} else if (ch == '\f') {
			os << "\\f";
		} else if (ch == '\n') {
			os << "\\n";
		} else if (ch == '\r') {
			os << "\\r";
		} else if (ch == '\t') {
			os << "\\t";
		} else if (ch < 32) {
			os
				<< "\\u00"
				<< hexNibble((unsigned char)ch >> 4)
				<< hexNibble((unsigned char)ch & 0x0f);
		} else {
			os << ch;
		}
	}
	os << '"';
}

static void writeBinary(const std::vector<unsigned char> &bin, std::ostream &os) {
	os << "\"HEX:";
	for (unsigned char ch: bin) {
		os
			<< hexNibble(ch >> 4)
			<< hexNibble(ch & 0x0f);
	}
	os << '"';
}

static void indent(int depth, std::ostream &os) {
	while (depth--) {
		os << "  ";
	}
}

static void writeValue(sbon::Reader r, std::ostream &os, int depth);

static void writeArray(sbon::ArrayReader r, std::ostream &os, int depth) {
	os << "[\n";
	while (r.hasNext()) {
		indent(depth + 1, os);
		writeValue(r.next(), os, depth + 1);

		if (r.hasNext()) {
			os << ',';
		}
		os << '\n';
	}
	indent(depth, os);
	os << ']';
}

static void writeObject(sbon::ObjectReader r, std::ostream &os, int depth) {
	os << "{\n";
	std::string key;
	while (r.hasNext()) {
		sbon::Reader val = r.next(key);

		indent(depth + 1, os);
		writeString(key, os);
		os << ": ";
		writeValue(val, os, depth + 1);

		if (r.hasNext()) {
			os << ',';
		}
		os << '\n';
	}

	indent(depth, os);
	os << '}';
}

static void writeValue(sbon::Reader r, std::ostream &os, int depth) {
	switch (r.getType()) {
	case sbon::Type::BOOL:
		if (r.getBool()) {
			os << "true";
		} else {
			os << "false";
		}
		break;

	case sbon::Type::NIL:
		os << "null";
		r.getNil();
		break;

	case sbon::Type::STRING:
		writeString(r.getString(), os);
		break;

	case sbon::Type::BINARY:
		writeBinary(r.getBinary(), os);
		break;

	case sbon::Type::FLOAT:
		os << r.getFloat();
		break;

	case sbon::Type::DOUBLE:
		os << r.getDouble();
		break;

	case sbon::Type::INT:
		os << r.getInt();
		break;

	case sbon::Type::UINT:
		os << r.getUInt();
		break;

	case sbon::Type::ARRAY:
		r.getArray([&](sbon::ArrayReader arr) {
			writeArray(arr, os, depth);
		});
		break;

	case sbon::Type::OBJECT:
		r.getObject([&](sbon::ObjectReader arr) {
			writeObject(arr, os, depth);
		});
		break;

	default:
		break;
	}
}

int main(int argc, char **argv) {
	std::istream *input;
	std::fstream infile;

	std::ostream *output;
	std::fstream outfile;

	if (argc == 1) {
		input = &std::cin;
		output = &std::cout;
	} else if (argc == 2) {
		infile.open(argv[1]);
		if (!infile) {
			std::cerr << "Couldn't open " << argv[1] << '\n';
			return 1;
		}
		input = &infile;
		output = &std::cout;
	} else if (argc == 3) {
		infile.open(argv[1]);
		if (!infile) {
			std::cerr << "Couldn't open " << argv[1] << '\n';
			return 1;
		}
		input = &infile;

		outfile.open(argv[2]);
		if (!outfile) {
			std::cerr << "Couldn't open " << argv[1] << '\n';
			return 1;
		}
		output = &outfile;
	} else {
		std::cout << "Usage: " << argv[0] << " [infile] [outfile]\n";
		return 1;
	}

	sbon::Reader reader(input);
	writeValue(reader, *output, 0);
	*output << '\n';
}
