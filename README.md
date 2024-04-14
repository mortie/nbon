# NBON: Non-Binary Object Notation

![NBON Logo](./NBON.png)

There's a lot of binary-JSON-style formats.
Naturally, they're all more or less unreadable in a text editor;
you more or less *need* a tool to inspect their contents,
and you can't reasonably hand-edit them.

NBON represents a sort of middle-ground between
a pure textual notation like JSON and
a pure binary notation like CBOR/BSON/MessagePack/UBJSON/BJSON/etc.

An NBON-encoded value can be one of the following:

* True: `'T'` (0x56)
* False: `'F'` (0x46)
* Null: `'N'` (0x4e)
* String: `'S'` (0x54), followed by a 0-terminated UTF-8-encoded string
* Float: `'f'` (0x66), followed by a little-endian IEEE 754 32-bit floating point number
* Double: `'d'` (0x64), followed by a little-endian IEEE 754 64-bit floating point number
* Binary data: `'b'` (0x62), followed by a positive unsigend LEB128 encoded value
  containing the number of bytes, followed by the bytes themselves
* Positive integer: `'+'` (0x2b), followed by a positive unsigned LEB128 encoded value
* Negative integer: `'-'` (0x2d), followed by a negative unsigned LEB128 encoded value
* Immediate integer: `'0'` (0x30) to `'9'` (0x39) represent the integers 0 to 9
* Array: `'['` (0x5b), followed by 0 or more ordered values, followed by `']'` (0x5d)
* Object: `'{'` (0x7b), followed by 0 or more ordered key-value pairs,
  followed by `'}'` (0x7d)

A key-value pair consists of a 0-terminated UTF-8-encoded string,
followed by an NBON-encoded value.

Here's an example NBON document, with non-textual characters represented using
hex notation in angle brackets:

```
{name<00>SBob<00>age<00>+<38>hobbies[Sbiking<00>Sjogging<00>]children<0>2}
```

This is equivalent to the following JSON:

```
{
    "name": "Bob",
    "age": 56,
    "hobbies": ["biking", "jogging"],
    "children": 2
}
```

## Semantics

* The values true, false and null have the same meaning they do in JSON.
* Floats and doubles are distinct. Applications are free to treat them interchangeably,
  but converting a float to a double does not preserve the semantic meaning of the document.
* Arrays are ordered (i.e they're lists, not sets).
  Changing the order of values in an array does not preserve the semantic meaning
  of the document.
* Objects are ordered. Applications are free to disregard their order where appropriate,
  but changing the order of key-value pairs in an object
  does not preserve the semantic meaning of the document.
* Immediate integers are semantically equivalent to positive integers.
  For example, changing a `'3'` to a `'+'` folowed by a LEB128-encoded number 3,
  or vice versa, **does** preserve the semantics of the document.
* The LEB128 encoding scheme for integers allows them to be arbitrarily large,
  but implementations may impose restrictions.
  Consumers of NBON documents must support positive integers up to 9007199254740991
  and negative integers down to -9007199254740991
  (this is the range supported by 64-bit floats).

## Comparison to JSON

The main advantage over JSON is simplicity: it's incredibly easy to parse and generate.
You don't need to escaped and de-escape strings (JSON is especially annoying here with
how high Unicode code points are encoded with two `\u` escapes using UTF-16
surrogate pairs), you don't need to serialize and parse floats (have you ever tried
writing a float serializer + parser which round-trips every number correctly?),
you don't need to worry about whitespace, etc.

As a result, NBON should be much faster than JSON.

The obvious drawback is that JSON is completely human readable.

## Comparison to other binary JSON variants

NBON is much more human readable than the others; you can look at an NBON
file in a text editor or `cat` it in a terminal and get a good impression of
the structure of the document.

NBON is also much simpler and thus easier to implement.
When I wrote [my MessagePack implementation](https://github.com/mortie/msgstream),
there were a lot of special cases which I got subtly wrong.
In comparison, anyone who's used to working with binary data should be able
to write an NBON implementation in a fairly short amount of time and more or
less get it right on the first try.

However, the other formats aren't just more complex and less human readable
for the fun of it: they usually use more encoding tricks to produce more
compact output.
For example, MessagePack can represent any integer between -32 and 127
in one byte, whereas NBON can only represent the integers 0-9 in 1 byte
and requires 2 bytes for the range -127 to 127.
(If encode size is very important though, something like
[Cap'n Proto](https://capnproto.org/) or [Protocol Buffers](https://protobuf.dev/)
might be a better idea, since all JSON-like formats waste a lot of space
by putting object keys everywhere.)

All the other encodings are also more widely supported than NBON.
