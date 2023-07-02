#pragma once

// stl includes
#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <sstream>

// custom includes
#include "chunk.h"
#include "pallete.h"
#include "pixel_reader.h"
#include "../../image.h"
#include "../utils.h"

Image ReadPng(std::string_view filename);

namespace png_decoder {

    class PNGDecoder {
    public:
        PNGDecoder(std::istream& stream);

        Image decode();
    
    private:
        struct IntermediateImage {
            uint32_t width;
            uint32_t height;
            // std::vector <uint8_t> data;
            std::vector <std::vector<uint8_t>> data;

            std::string to_string() const;
        };
        struct Header {
            uint32_t width;
            uint32_t height;
            uint8_t bit_depth;
            uint8_t color_type;
            uint8_t compression_method;
            uint8_t filter_method;
            uint8_t interlace_method;

            std::string to_string() const;
            bool is_greyscale() const noexcept;
            bool is_greyscale_with_alpha() const noexcept;
            bool is_rgb() const noexcept;
            bool is_rgb_with_alpha() const noexcept;
            bool is_pallete_indexed() const noexcept;
        };

        

        uint64_t read_png_signature();
        void validate_png_signature_valid(uint64_t signature) const;
        
        void read_all_chunks();
        std::optional<Chunk> read_chunk();
        void validate_chunks();
        void validate_chunks_crc_checksum();

        void read_header();
        void validate_header();

        void read_pallete();
        void validate_pallete();

        void inflate_data_chunks();

        std::vector <IntermediateImage> defilter();
        IntermediateImage defilter_non_interlaced(uint32_t width, uint32_t height, uint32_t& current_position);
        std::vector <IntermediateImage> defilter_interlaced();

        uint32_t bits_per_pixel() const;
        void set_subimage_size(uint32_t pass, uint32_t &w, uint32_t &h);
        void set_image_pos_by_subimage(size_t w, size_t h, int pass, size_t& W, size_t& H);

        Image create_image(std::vector <IntermediateImage>& parts);
        PixelReader::PixelType get_pixel_type() const;


    private:
        const inline static uint64_t PNG_SIGNATURE_VALUE = utils::convert_from_big_endian_to_host((uint64_t) (0x0a1a0a0d474e5089));
    
        std::istream& m_stream;
        std::vector <Chunk> m_chunks;
        std::vector <uint8_t> m_image_data;
        Header m_header;
        Pallete m_pallete;
    };

} // namesapce png_decoder