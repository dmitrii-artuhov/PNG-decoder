#include "defilter.h"

// stl includes
#include <cstdint>
#include <memory>

// custom includes
#include "../utils.h"



namespace png_decoder {

    uint32_t Defilter::mod(uint32_t value) {
        return (value % MOD + MOD) % MOD;
    }

    uint32_t Defilter::prev(const Scanline& scanline, uint32_t pos, uint32_t bpp) {
        return pos < bpp ? 0 : static_cast<uint32_t> (scanline.data[pos - bpp]);
    }


    // defilters

    // NONE
    void None::apply([[maybe_unused]] Scanline& scanline, [[maybe_unused]] const Scanline& previous_defiltered_scanline, [[maybe_unused]] uint32_t bpp) const {
        // no-op
    }

    // SUB
    void Sub::apply(Scanline& scanline, [[maybe_unused]] const Scanline& previous_defiltered_scanline, uint32_t bpp) const {
        for (size_t i = 0; i < scanline.data.size(); ++i) {
            uint32_t previous = prev(scanline, i, bpp);
            uint32_t current = scanline.data[i];

            scanline.data[i] = mod(current + previous);
        }
    }


    // UP
    void Up::apply(Scanline& scanline, const Scanline& previous_defiltered_scanline, [[maybe_unused]] uint32_t bpp) const {
        for (size_t i = 0; i < scanline.data.size(); ++i) {
            uint32_t prior = previous_defiltered_scanline.data[i];
            uint32_t current = scanline.data[i];

            scanline.data[i] = mod(current + prior);
        }
    }


    // AVERAGE
    void Average::apply(Scanline& scanline, const Scanline& previous_defiltered_scanline, uint32_t bpp) const {
        for (size_t i = 0; i < scanline.data.size(); ++i) {
            uint32_t previous = prev(scanline, i, bpp);
            uint32_t prior = previous_defiltered_scanline.data[i];
            uint32_t current = scanline.data[i];

            scanline.data[i] = mod(current + (previous + prior) / 2);
        }
    }


    // PAETH
    void Paeth::apply(Scanline& scanline, const Scanline& previous_defiltered_scanline, uint32_t bpp) const {
        for (size_t i = 0; i < scanline.data.size(); ++i) {
            uint32_t left = prev(scanline, i, bpp);
            uint32_t above = previous_defiltered_scanline.data[i];
            uint32_t upper_left = prev(previous_defiltered_scanline, i, bpp);
            uint32_t current = scanline.data[i];

            scanline.data[i] = current + mod(predictor(left, above, upper_left));
        }
    }

    int32_t Paeth::predictor(int32_t left, int32_t above, int32_t upper_left) {
        // a = left, b = above, c = upper left
        int32_t p = left + above - upper_left;
        int32_t pa = std::abs(p - left);
        int32_t pb = std::abs(p - above);
        int32_t pc = std::abs(p - upper_left);

        // return nearest of left, above, upper left
        // breaking ties in order left, above, upper left
        if (pa <= pb && pa <= pc) {
            return left;
        }
        if (pb <= pc) {
            return above;
        }
        return upper_left;
    }


    std::unique_ptr<Defilter> Defilter::create_defilter(uint8_t filter_type) {
        if (filter_type == static_cast<uint8_t> (FilterType::NONE)) {
            return std::make_unique<None>();
        }
        else if (filter_type == static_cast<uint8_t> (FilterType::SUB)) {
            return std::make_unique<Sub>();
        }
        else if (filter_type == static_cast<uint8_t> (FilterType::UP)) {
            return std::make_unique<Up>();
        }
        else if (filter_type == static_cast<uint8_t> (FilterType::AVERAGE)) {
            return std::make_unique<Average>();
        }
        else if (filter_type == static_cast<uint8_t> (FilterType::PAETH)) {
            return std::make_unique<Paeth>();
        }

        throw ::error::invalid_arguments("Defilter::create_filter: invalid `filter_type`: " + std::to_string(filter_type));
    }
}