# Png Decoder

## Problem

In this task you should implement your own decoder of png images. PNG is one of the most popular formats
for lossless image compression and one of the simplest. It supports grayscale images, regular rgb, indexed images (with a fixed palette) and also
can contain alpha-channel (transparency).

Note, that you'll have to spend some time on digging into specification, so plan your time accordingly. However, the specification is very
well written, and this is one of rare cases when you don't even need additional materials besides it.

Here is the specification: http://www.libpng.org/pub/png/spec/1.2/PNG-Contents.html.

However, you don't need to read it all, since the goal here is only to support the most important parts of png. So you may only read chapters 1-3, 4.1 (ancillary chunks are skipped in this task), 5 and 6.

Once you've read it, this pipeline of decoding should be pretty straightforward:

1. Read signature bytes and validate it.
2. Read chunks until you get IEND chunk. Validate CRC of each chunk.
3. Save the information provided by IHDR and PLTE chunks, since you'll need it later.
4. Concatenate the content of all IDAT chunks into a single byte vector (lets call it `B`). You don't need to implement decoding on the fly in this task.
5. Once you've reached IEND, you can start decoding `B`. Use deflate to decompress `B` into another byte vector `R`.
6. Process `R` as described in the specification. Namely, you should process `R` by scanlines, applying specified filters. For non-interlaced images you need to do only 1 pass. In interlaced images, all 7 passes are concatenated in `R`.

## Implementation details and scoring

This tasks will also be reviewed manually to check whether your implementation follows some requirements specified here.
Your decoder should support all possible combination of modes (however, you may only support bit depth <= 8), specified in IHDR (this is checked in tests).

You are free to organize files as you want, however you should define `build.cmake`, where decoder is built. This file should define
`PNG_STATIC` variable, which contains all the libraries needed to link (check out `CMakeLists.txt` for details).

The entry point is `Image ReadPng(std::string_view filename)` function in `png_decoder.h`.

You'll have to use 2 external libraries for deflate and crc calculation. The following libraries are installed in the testing system:
1. zlib1g-dev (zlib)
2. https://github.com/ebiggers/libdeflate (you can use either 1 or 2 for deflate)
3. boost (for CRC)

These are the requirements for your decoder (**bold notes** indicate penalties for not implementing them):

1. Decoder should be implemented as a class, receiving `std::istream&` (assume its opened in a binary mode) in it's constructor (**mandatory**).
2. Deflate logic should be completely separated from the decoder. Both libraries given here are C libraries, so you should create a RAII wrapper
around the selected library and use it inside your decoder (**mandatory**).
3. Boost is rather heavy sometimes to include, so separate it from your decoder. Namely, crc calculation should be placed inside a separate object file
and decoder itself shouldn't include boost as dependency at all. (**-200**)
4. Carefully handle various errors. For a example, you should validate crc even for ancillary chunks, despite the fact you skip them.
Don't forget to check for EOF when you read from the input stream. In other words, your decoder shouldn't segfault on any input sequence. Throw exceptions for invalid png images. (**-200**)
5. Sometimes you'll need to operate on bits and not bytes (for a example, when bit depth < 8). It might be a good idea to create special `BitReader` (or Writer), which takes a sequence of bytes and can read it bitwise.


## Tests

Note, that you can specify second argument in any call to `CheckImage` in `test.cpp`. This way you can write the output of your decoder into
png file. The path there is relative to cwd of test executable (so its most likely your build directory).
