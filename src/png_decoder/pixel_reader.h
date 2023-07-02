#pragma once


// stl includes
#include <cstdint>
#include <memory>
#include <vector>
#include <optional>

// custom includes
#include "../../image.h"
#include "pallete.h"

namespace png_decoder {

    class PixelReader {
    public:
        enum class PixelType : uint8_t {
            GREYSCALE = 0,
            RGB = 1,
            PALLETE = 2,
            GRAYSCALE_WITH_ALPHA = 3,
            RGB_WITH_ALPHA = 4
        };

        PixelReader(uint8_t bit_depth, Pallete& pallete);
        static std::unique_ptr<PixelReader> create_pixel_reader(PixelType pixel_type, uint8_t bit_depth, Pallete& pallete);

        virtual std::optional<RGB> get_pixel_at(std::vector<uint8_t>& data, size_t position, size_t bits_per_pixel) = 0;
        virtual ~PixelReader() = default;
    protected:
        uint8_t m_bit_depth;
        Pallete& m_pallete;
    };

    class RGBPixelReader : public PixelReader {
    public:
        RGBPixelReader(uint8_t bit_depth, Pallete& pallete);
        virtual std::optional<RGB> get_pixel_at(std::vector<uint8_t>& data, size_t position, size_t bits_per_pixel) override;
    };

    
    class RGBWithAlphaPixelReader : public PixelReader {
    public:
        RGBWithAlphaPixelReader(uint8_t bit_depth, Pallete& pallete);
        virtual std::optional<RGB> get_pixel_at(std::vector<uint8_t>& data, size_t position, size_t bits_per_pixel) override;
    };

    class GreyScalePixelReader : public PixelReader {
    public:
        GreyScalePixelReader(uint8_t bit_depth, Pallete& pallete);
        virtual std::optional<RGB> get_pixel_at(std::vector<uint8_t>& data, size_t position, size_t bits_per_pixel) override;
    };


    class GreyScaleWithAlphaPixelReader : public PixelReader {
    public:
        GreyScaleWithAlphaPixelReader(uint8_t bit_depth, Pallete& pallete);
        virtual std::optional<RGB> get_pixel_at(std::vector<uint8_t>& data, size_t position, size_t bits_per_pixel) override;
    };

    class PalletePixelReader : public PixelReader {
    public:
        PalletePixelReader(uint8_t bit_depth, Pallete& pallete);
        virtual std::optional<RGB> get_pixel_at(std::vector<uint8_t>& data, size_t position, size_t bits_per_pixel) override;
    };

}
