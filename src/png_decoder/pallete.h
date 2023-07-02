#pragma once


// stl includes
#include <cstdint>
#include <string>
#include <vector>

// custom includes

namespace png_decoder {
    
    struct PalleteColor {
        uint8_t red;
        uint8_t green;
        uint8_t blue;

        std::string to_string() const;
    };
    
    struct Pallete {
        std::vector <PalleteColor> entries;

        std::string to_string(bool should_print_entries) const;
    };

} // namespace png_decoder
