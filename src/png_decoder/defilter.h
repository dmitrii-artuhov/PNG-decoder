#pragma once

// stl includes
#include <cstdint>
#include <vector>
#include <memory>

// custom includes

namespace png_decoder {
    struct Scanline {
        Scanline(uint8_t filter_type_, uint32_t data_length) {
            filter_type = filter_type_;
            data.resize(data_length, 0);
        }

        uint8_t filter_type;
        std::vector <uint8_t> data;
    };
    
    class Defilter {
    public:
        static std::unique_ptr<Defilter> create_defilter(uint8_t filter_type);
        // bpp is a number of bytes per pixel
        virtual void apply(Scanline& scanline, const Scanline& previous_defiltered_scanline, uint32_t bpp) const = 0;
        virtual ~Defilter() = default;

    protected:
        enum class FilterType : uint8_t {
            NONE = 0,
            SUB = 1,
            UP = 2,
            AVERAGE = 3,
            PAETH = 4
        };
        static uint32_t mod(uint32_t value);
        static uint32_t prev(const Scanline& scanline, uint32_t pos, uint32_t bpp);

    private:
        static constexpr uint32_t MOD = 256;
    };

    class None : public Defilter {
    public:
        void apply(Scanline& scanline, const Scanline& previous_defiltered_scanline, uint32_t bpp) const override;
    };
        
    class Sub : public Defilter {
    public:
        void apply(Scanline& scanline, const Scanline& previous_defiltered_scanline, uint32_t bpp) const override;
    };


    class Up : public Defilter {
    public:
        void apply(Scanline& scanline, const Scanline& previous_defiltered_scanline, uint32_t bpp) const override;
    };


    class Average : public Defilter {
    public:
        void apply(Scanline& scanline, const Scanline& previous_defiltered_scanline, uint32_t bpp) const override;
    };


    class Paeth : public Defilter {
    public:
        void apply(Scanline& scanline, const Scanline& previous_defiltered_scanline, uint32_t bpp) const override;

    private:
        static int32_t predictor(int32_t a, int32_t b, int32_t c);
    };


}