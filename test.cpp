#include <catch.hpp>
#include "test_commons.hpp"

TEST_CASE("logo") {
    CheckImage("logo.png");
}

TEST_CASE("grayscale") {
    CheckImage("lenna_grayscale.png");
}

TEST_CASE("index") {
    CheckImage("lenna_index.png");
}

TEST_CASE("alpha") {
    CheckImage("logo_alpha.png");
}

TEST_CASE("bit") {
    CheckImage("1.png");
}

TEST_CASE("interlace") {
    CheckImage("inter.png");
}

TEST_CASE("grayscale_alpha_interlace") {
    CheckImage("alpha_grayscale.png");
}

TEST_CASE("bad_crc") {
    CHECK_THROWS(CheckImage("crc.png"));
}
