#pragma once

// stl includes
#include <cstdint>

// custom includes

namespace png_decoder {
    struct BitReader {
        static int get_value_from_byte(uint8_t byte, uint8_t block_position_inside_byte, uint8_t bits_count_in_block);
    };
} // namespace pnd_decoder
