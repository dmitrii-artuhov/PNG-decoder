#pragma once

// stl includes
#include <boost/crc.hpp>

// custom includes



namespace png_decoder::crc_calculator {
    static inline const auto BITS_COUNT = 32;
    static inline const auto TRUNC_POLICY = 0x4C11DB7;
    static inline const auto INITIAL_REMAINDER = 0xFFFFFFFF;
    static inline const auto FINAL_XOR = 0xFFFFFFFF;
    static inline const auto LOWEST_TO_HIGHEST = true;
    static inline const auto REMAINDER_BEFORE_XOR = true;

    uint32_t get_crc32_checksum(const char* bytes, uint32_t length);

}