#include "pallete.h"

// stl includes
#include <sstream>

// custom includes

namespace png_decoder {

    std::string PalleteColor::to_string() const {
        return "PalleteColor(r=" + std::to_string(red) + ", g=" + std::to_string(green) + ", b=" + std::to_string(blue) +")";
    }

    std::string Pallete::to_string(bool should_print_entries) const {
        std::stringstream ss;
        ss << "======== PNG Pallete ========" << std::endl;
        ss << "Entries count: " << entries.size() << std::endl;
        if (should_print_entries) {
            ss << "Colors: " << std::endl;

            for (auto& rgb : entries) {
                ss << rgb.to_string() << std::endl;
            }
        }
        return ss.str();
    }
}