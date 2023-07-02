#include "pixel_reader.h"

// stl includes
#include <cstdint>
#include <memory>
#include <vector>
#include <optional>

// custom includes
#include "../errors.h"
#include "../../image.h"
#include "../utils.h"
#include "bit_reader.h"
#include "pallete.h"


namespace png_decoder {

    PixelReader::PixelReader(uint8_t bit_depth, Pallete& pallete):
        m_bit_depth(bit_depth),
        m_pallete(pallete) {}
    
    std::unique_ptr<PixelReader> PixelReader::create_pixel_reader(PixelType pixel_type, uint8_t bit_depth, Pallete& pallete) {
        switch (pixel_type) {
            case PixelType::RGB: {
                // std::cout << "PixelReader type is RGB" << std::endl;
                return std::make_unique<RGBPixelReader> (bit_depth, pallete);
            }
            case PixelType::RGB_WITH_ALPHA: {
                // std::cout << "PixelReader type is RGB_WITH_ALPHA" << std::endl;
                return std::make_unique<RGBWithAlphaPixelReader> (bit_depth, pallete);
            }
            case PixelType::GREYSCALE: {
                // std::cout << "PixelReader type is GREYSCALE" << std::endl;
                return std::make_unique<GreyScalePixelReader> (bit_depth, pallete);
            }
            case PixelType::GRAYSCALE_WITH_ALPHA: {
                // std::cout << "PixelReader type is GRAYSCALE_WITH_ALPHA" << std::endl;
                return std::make_unique<GreyScaleWithAlphaPixelReader> (bit_depth, pallete);
            }
            case PixelType::PALLETE: {
                // std::cout << "PixelReader type is PALLETE" << std::endl;
                return std::make_unique<PalletePixelReader> (bit_depth, pallete);
            }
        }
    }

    // RGB
    RGBPixelReader::RGBPixelReader(uint8_t bit_depth, Pallete& pallete): PixelReader(bit_depth, pallete) {}

    std::optional<RGB> RGBPixelReader::get_pixel_at(std::vector<uint8_t>& data, size_t index, size_t bits_per_pixel) {
        size_t position = index * (bits_per_pixel / 8);
        
        if (position >= data.size()) {
            return std::nullopt;
        }

        int alpha = (1 << m_bit_depth) - 1; // fully opaque

        if (m_bit_depth == 8) {
            uint8_t red;
            uint8_t green;
            uint8_t blue;

            utils::read_data_as_host_endian(
                reinterpret_cast<unsigned char*> (data.data() + position),
                &red,
                sizeof(red),
                "Unable to read red channel of pixel at position " + std::to_string(position)
            );

            position += sizeof(red);


            utils::read_data_as_host_endian(
                reinterpret_cast<unsigned char*> (data.data() + position),
                &green,
                sizeof(green),
                "Unable to read green channel of pixel at position " + std::to_string(position)
            );

            position += sizeof(green);


            utils::read_data_as_host_endian(
                reinterpret_cast<unsigned char*> (data.data() + position),
                &blue,
                sizeof(blue),
                "Unable to read blue channel of pixel at position " + std::to_string(position)
            );

            position += sizeof(blue);

            return std::make_optional<RGB> (RGB{ red, green, blue, alpha });
        }
        else if (m_bit_depth == 16) {
            uint16_t red;
            uint16_t green;
            uint16_t blue;

            utils::read_data_as_big_endian_and_convert_to_host_endianess(
                reinterpret_cast<unsigned char*> (data.data() + position),
                &red,
                sizeof(red),
                "Unable to read red channel of pixel at position " + std::to_string(position)
            );

            position += sizeof(red);


            utils::read_data_as_big_endian_and_convert_to_host_endianess(
                reinterpret_cast<unsigned char*> (data.data() + position),
                &green,
                sizeof(green),
                "Unable to read green channel of pixel at position " + std::to_string(position)
            );

            position += sizeof(green);


            utils::read_data_as_big_endian_and_convert_to_host_endianess(
                reinterpret_cast<unsigned char*> (data.data() + position),
                &blue,
                sizeof(blue),
                "Unable to read blue channel of pixel at position " + std::to_string(position)
            );

            position += sizeof(blue);

            return std::make_optional<RGB> (RGB{ red, green, blue, alpha });
        }
        
        throw ::error::invalid_arguments("m_bit_depth must be 8 or 16");
    }


    // RGB with alpha
    RGBWithAlphaPixelReader::RGBWithAlphaPixelReader(uint8_t bit_depth, Pallete& pallete): PixelReader(bit_depth, pallete) {}

    std::optional<RGB> RGBWithAlphaPixelReader::get_pixel_at(std::vector<uint8_t>& data, size_t index, size_t bits_per_pixel) {
        size_t position = index * (bits_per_pixel / 8);
        
        if (position >= data.size()) {
            return std::nullopt;
        }


        if (m_bit_depth == 8) {
            uint8_t red;
            uint8_t green;
            uint8_t blue;
            uint8_t alpha;

            utils::read_data_as_host_endian(
                reinterpret_cast<unsigned char*> (data.data() + position),
                &red,
                sizeof(red),
                "Unable to read red channel of pixel at position " + std::to_string(position)
            );

            position += sizeof(red);


            utils::read_data_as_host_endian(
                reinterpret_cast<unsigned char*> (data.data() + position),
                &green,
                sizeof(green),
                "Unable to read green channel of pixel at position " + std::to_string(position)
            );

            position += sizeof(green);


            utils::read_data_as_host_endian(
                reinterpret_cast<unsigned char*> (data.data() + position),
                &blue,
                sizeof(blue),
                "Unable to read blue channel of pixel at position " + std::to_string(position)
            );

            position += sizeof(blue);


            utils::read_data_as_host_endian(
                reinterpret_cast<unsigned char*> (data.data() + position),
                &alpha,
                sizeof(alpha),
                "Unable to read alpha channel of pixel at position " + std::to_string(position)
            );

            position += sizeof(alpha);

            return std::make_optional<RGB> (RGB{ red, green, blue, alpha });
        }
        else if (m_bit_depth == 16) {
            uint16_t red;
            uint16_t green;
            uint16_t blue;
            uint16_t alpha;

            utils::read_data_as_big_endian_and_convert_to_host_endianess(
                reinterpret_cast<unsigned char*> (data.data() + position),
                &red,
                sizeof(red),
                "Unable to read red channel of pixel at position " + std::to_string(position)
            );

            position += sizeof(red);


            utils::read_data_as_big_endian_and_convert_to_host_endianess(
                reinterpret_cast<unsigned char*> (data.data() + position),
                &green,
                sizeof(green),
                "Unable to read green channel of pixel at position " + std::to_string(position)
            );

            position += sizeof(green);


            utils::read_data_as_big_endian_and_convert_to_host_endianess(
                reinterpret_cast<unsigned char*> (data.data() + position),
                &blue,
                sizeof(blue),
                "Unable to read blue channel of pixel at position " + std::to_string(position)
            );

            position += sizeof(blue);

            
            utils::read_data_as_big_endian_and_convert_to_host_endianess(
                reinterpret_cast<unsigned char*> (data.data() + position),
                &alpha,
                sizeof(alpha),
                "Unable to read alpha channel of pixel at position " + std::to_string(position)
            );

            position += sizeof(alpha);

            return std::make_optional<RGB> (RGB{ red, green, blue, alpha });
        }

        
        throw ::error::invalid_arguments("m_bit_depth must be 8 or 16");
    }


    // Greysacle 
    GreyScalePixelReader::GreyScalePixelReader(uint8_t bit_depth, Pallete& pallete): PixelReader(bit_depth, pallete) {}

    std::optional<RGB> GreyScalePixelReader::get_pixel_at(std::vector<uint8_t>& data, size_t index, size_t bits_per_pixel) {
        size_t position;
        int alpha = (1 << m_bit_depth) - 1; // fully opaque

        if (m_bit_depth < 8) {
            position = (index * bits_per_pixel) / 8;
            if (position >= data.size()) {
                return std::nullopt;
            }
            
            uint8_t block_position_in_byte = index % (8 / bits_per_pixel);
            
            int grey_scale = BitReader::get_value_from_byte(
                data[position],
                block_position_in_byte,
                m_bit_depth
            );

            return std::make_optional<RGB> (RGB{ grey_scale, grey_scale, grey_scale, alpha });
        }
        else if (m_bit_depth == 8) {
            position = index * (bits_per_pixel / 8);

            if (position >= data.size()) {
                return std::nullopt;
            }

            uint8_t grey_scale;

            utils::read_data_as_host_endian(
                reinterpret_cast<unsigned char*> (data.data() + position),
                &grey_scale,
                sizeof(grey_scale),
                "Unable to read grey scale channel of pixel at position " + std::to_string(position)
            );

            position += sizeof(grey_scale);

            return std::make_optional<RGB> (RGB{ grey_scale, grey_scale, grey_scale, alpha });
        }
        else if (m_bit_depth == 16) {    
            position = index * (bits_per_pixel / 8);

            if (position >= data.size()) {
                return std::nullopt;
            }
            
            uint16_t grey_scale;

            utils::read_data_as_big_endian_and_convert_to_host_endianess(
                reinterpret_cast<unsigned char*> (data.data() + position),
                &grey_scale,
                sizeof(grey_scale),
                "Unable to read grey scale channel of pixel at position " + std::to_string(position)
            );

            position += sizeof(grey_scale);

            return std::make_optional<RGB> (RGB{ grey_scale, grey_scale, grey_scale, alpha });
        }

        throw ::error::invalid_arguments("m_bit_depth must be 8 or 16");
    }


    // Greysacle with alpha
    GreyScaleWithAlphaPixelReader::GreyScaleWithAlphaPixelReader(uint8_t bit_depth, Pallete& pallete): PixelReader(bit_depth, pallete) {}

    std::optional<RGB> GreyScaleWithAlphaPixelReader::get_pixel_at(std::vector<uint8_t>& data, size_t index, size_t bits_per_pixel) {
        size_t position = index * (bits_per_pixel / 8);
        if (position >= data.size()) {
            return std::nullopt;
        }

        if (m_bit_depth == 8) {
            uint8_t grey_scale;
            uint8_t alpha;

            utils::read_data_as_host_endian(
                reinterpret_cast<unsigned char*> (data.data() + position),
                &grey_scale,
                sizeof(grey_scale),
                "Unable to read grey scale channel of pixel at position " + std::to_string(position)
            );

            position += sizeof(grey_scale);

            utils::read_data_as_host_endian(
                reinterpret_cast<unsigned char*> (data.data() + position),
                &alpha,
                sizeof(alpha),
                "Unable to read alpha channel of pixel at position " + std::to_string(position)
            );

            position += sizeof(alpha);

            return std::make_optional<RGB> (RGB{ grey_scale, grey_scale, grey_scale, alpha });
        }
        else if (m_bit_depth == 16) {
            uint16_t grey_scale;
            uint16_t alpha;

            utils::read_data_as_big_endian_and_convert_to_host_endianess(
                reinterpret_cast<unsigned char*> (data.data() + position),
                &grey_scale,
                sizeof(grey_scale),
                "Unable to read grey scale channel of pixel at position " + std::to_string(position)
            );

            position += sizeof(grey_scale);
            
            utils::read_data_as_big_endian_and_convert_to_host_endianess(
                reinterpret_cast<unsigned char*> (data.data() + position),
                &alpha,
                sizeof(alpha),
                "Unable to read alpha channel of pixel at position " + std::to_string(position)
            );

            position += sizeof(alpha);

            return std::make_optional<RGB> (RGB{ grey_scale, grey_scale, grey_scale, alpha });
        }

        throw ::error::invalid_arguments("m_bit_depth must be 8 or 16");
    }


    // Pallete
    PalletePixelReader::PalletePixelReader(uint8_t bit_depth, Pallete& pallete): PixelReader(bit_depth, pallete) {}

    std::optional<RGB> PalletePixelReader::get_pixel_at(std::vector<uint8_t>& data, size_t index, size_t bits_per_pixel) {
        size_t position;
        int alpha = (1 << 8) - 1; // fully opaque, for pallete each color sergment is 1 byte

        if (m_bit_depth < 8) {
            position = (index * bits_per_pixel) / 8;
            if (position >= data.size()) {
                return std::nullopt;
            }

            
            uint8_t block_position_in_byte = index % (8 / bits_per_pixel);
            
            int index_in_pallete_table = BitReader::get_value_from_byte(
                data[position],
                block_position_in_byte,
                m_bit_depth
            );

            PalleteColor color = m_pallete.entries[index_in_pallete_table];
            return std::make_optional<RGB> (RGB{ color.red, color.green, color.blue, alpha });
        }
        else if (m_bit_depth == 8) {
            position = index * (bits_per_pixel / 8);
            if (position >= data.size()) {
                return std::nullopt;
            }

            uint8_t index_in_pallete_table;

            utils::read_data_as_host_endian(
                reinterpret_cast<unsigned char*> (data.data() + position),
                &index_in_pallete_table,
                sizeof(index_in_pallete_table),
                "Unable to read pallete index at position " + std::to_string(position)
            );

            position += sizeof(index_in_pallete_table);
            
            PalleteColor color = m_pallete.entries[index_in_pallete_table];

            return std::make_optional<RGB> (RGB{ color.red, color.green, color.blue, alpha });
        }

        throw ::error::invalid_arguments("m_bit_depth must be 1, 2, 4 or 8");
    }


} // namespace png_decoder

