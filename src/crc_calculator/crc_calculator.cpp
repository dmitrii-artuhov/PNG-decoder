#include "crc_calculator.h"

// stl includes
#include <iostream>

namespace png_decoder::crc_calculator {

    
    uint32_t get_crc32_checksum(const char* bytes, uint32_t length) {
        // Compute the CRC of the chunk data and validate it
        boost::crc_optimal<BITS_COUNT, TRUNC_POLICY, INITIAL_REMAINDER, FINAL_XOR, LOWEST_TO_HIGHEST, REMAINDER_BEFORE_XOR> crc;
        crc.process_bytes(bytes, length);
        return crc.checksum();
    }

}