#include "bit_reader.h"


namespace png_decoder {
    
    int BitReader::get_value_from_byte(uint8_t byte, uint8_t block_position_inside_byte, uint8_t bits_count_in_block) {
        // number_of_blocks_inside_byte == m
        // block_position_inside_byte == p
        // bits_count_in_block == k

        int number_of_blocks_inside_byte = 8 / bits_count_in_block;

        int result = (
            byte >> (bits_count_in_block * (number_of_blocks_inside_byte - 1 - block_position_inside_byte))
        );

        result &= ((1 << bits_count_in_block) - 1);

        return result;
    }

} // namespace png_decoder
