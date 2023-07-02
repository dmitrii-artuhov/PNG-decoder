#include "png_decoder.h"

// stl includes
#include <iostream>
#include <fstream>
#include <string>
#include <optional>
#include <algorithm>
#include <set>

// custom includes
#include "../errors.h"
#include "../utils.h"
#include "../crc_calculator/crc_calculator.h"
#include "../inflater/inflater.h"
#include "defilter.h"
#include "pixel_reader.h"

Image ReadPng(std::string_view filename) {
    std::ifstream input_stream(filename.data(), std::ios_base::binary | std::ios_base::in);

    if (!input_stream || !input_stream.is_open()) {
        throw error::unable_to_open_file(std::string(filename));
    }

    png_decoder::PNGDecoder decoder = png_decoder::PNGDecoder(input_stream);
    Image img = decoder.decode();

    input_stream.close();

    return img;
}

namespace png_decoder {

    PNGDecoder::PNGDecoder(std::istream& stream): m_stream(stream), m_header({}), m_pallete({}) {}

    Image PNGDecoder::decode() {
        // signature
        validate_png_signature_valid(read_png_signature());
        
        // chunks
        read_all_chunks();
        validate_chunks();

        // read header
        read_header();
        validate_header();
        if (m_header.is_pallete_indexed()) {
            read_pallete();
            validate_pallete();
        }

        // inflation
        inflate_data_chunks();

        // defilter
        auto intermediate_images = defilter();

        // create image
        Image result = create_image(intermediate_images);

        return result;
    }

    uint64_t PNGDecoder::read_png_signature() {
        uint64_t signature;
        
        utils::read_stream_as_big_endian_and_convert_to_host_endianess(
            m_stream,
            &signature,
            sizeof(signature),
            "Cannot read png signature"
        );
  
        return signature;
    }

    void PNGDecoder::validate_png_signature_valid(uint64_t signature) const {
        if (!(PNG_SIGNATURE_VALUE == signature)) {
            throw error::invalid_png_signature();
        }
    }
    
    void PNGDecoder::read_all_chunks() {
        while (true) {
            std::optional<Chunk> chunk = read_chunk();
            if (!chunk.has_value()) {
                // EOF happend
                break;
            }
            // std::cout << chunk.value().to_string(false) << std::endl;
            m_chunks.push_back(chunk.value());
        }

        // std::cout << "Total chunks: " << m_chunks.size() << std::endl;
    }

    std::optional<Chunk> PNGDecoder::read_chunk() {
        uint32_t length;

        bool is_read = utils::read_stream_as_big_endian_and_convert_to_host_endianess(
            m_stream,
            &length,
            sizeof(length),
            "Cannot read chunk length at pos " + std::to_string(m_stream.tellg())
        );

        if (!is_read) {
            // EOF happended
            return std::nullopt;
            // further in the code will be an exception and not the regular EOF event
        }

        char type[5] = {0};
        std::vector <uint8_t> data(length);
        uint32_t crc;
        
        utils::read_as_host_endian(
            m_stream,
            &type,
            sizeof(type) - 1,
            "Cannot read chunk type at pos " + std::to_string(m_stream.tellg())
        );

        utils::read_as_host_endian(
            m_stream,
            data.data(),
            data.size(),
            "Cannot read chunk data at pos " + std::to_string(m_stream.tellg())
        );

        utils::read_stream_as_big_endian_and_convert_to_host_endianess(
            m_stream,
            &crc,
            sizeof(crc),
            "Cannot read chunk crc at pos " + std::to_string(m_stream.tellg())
        );

        return std::make_optional<Chunk> (Chunk(std::string(type), data, crc));
    }

    void PNGDecoder::validate_chunks() {
        // TODO: complete the validation

        // - IHDR comes first
        if (m_chunks.size() == 0 || m_chunks[0].get_type() != Chunk::ChunkType::HEADER) {
            throw error::invalid_header_chunk("Header chunk must come first");
        }
    

        // - IEND comes last and has zero length

        // - validate CRC
        validate_chunks_crc_checksum();

        // - run through all chunks and call if !chunk.is_valid(): throw
        // - IDAT chunks come one after another
    }

    void PNGDecoder::validate_chunks_crc_checksum() {
        for (size_t i = 0; i < m_chunks.size(); i++) {
            auto& chunk = m_chunks[i];
            auto checksum = crc_calculator::get_crc32_checksum(chunk.get_crc_bytes_sequence().data(), chunk.get_crc_bytes_sequence_length());

            if (chunk.get_crc() != checksum) {
                // std::cout << "Found invalid crc chunk" << std::endl;
                throw error::invalid_crc_checksum("Checksum: " + std::to_string(checksum) + ", chunk number " + std::to_string(i) + ": " + chunk.to_string(true));
            }
        }
    }


    void PNGDecoder::read_header() {
        Chunk& header_chunk = m_chunks[0];
        uint32_t offset = 0;

        // width
        utils::read_data_as_big_endian_and_convert_to_host_endianess(
            header_chunk.get_data_bytes() + offset,
            &m_header.width,
            sizeof(m_header.width),
            "Unable to read header chunk width"
        );

        offset += sizeof(m_header.width);


        // height
        utils::read_data_as_big_endian_and_convert_to_host_endianess(
            header_chunk.get_data_bytes() + offset,
            &m_header.height,
            sizeof(m_header.height),
            "Unable to read header chunk height"
        );

        offset += sizeof(m_header.height);

        // bit depth
        utils::read_data_as_host_endian(
            header_chunk.get_data_bytes() + offset,
            &m_header.bit_depth,
            sizeof(m_header.bit_depth),
            "Unable to read header chunk bit depth"
        );

        offset += sizeof(m_header.bit_depth);

        // color type
        utils::read_data_as_host_endian(
            header_chunk.get_data_bytes() + offset,
            &m_header.color_type,
            sizeof(m_header.color_type),
            "Unable to read header chunk color type"
        );

        offset += sizeof(m_header.color_type);

        // compression method
        utils::read_data_as_host_endian(
            header_chunk.get_data_bytes() + offset,
            &m_header.compression_method,
            sizeof(m_header.compression_method),
            "Unable to read header chunk compression method"
        );

        offset += sizeof(m_header.compression_method);

        // filter method
        utils::read_data_as_host_endian(
            header_chunk.get_data_bytes() + offset,
            &m_header.filter_method,
            sizeof(m_header.filter_method),
            "Unable to read header chunk filter method"
        );

        offset += sizeof(m_header.filter_method);

        // interlace method
        utils::read_data_as_host_endian(
            header_chunk.get_data_bytes() + offset,
            &m_header.interlace_method,
            sizeof(m_header.interlace_method),
            "Unable to read header chunk interlace method"
        );

        offset += sizeof(m_header.interlace_method);

        // std::cout << m_header.to_string() << std::endl;
    }

    void PNGDecoder::validate_header() {
        // Validate color type and bit depth

        auto is_value_in_set = [](int value, const std::set <int>& accept) -> bool {
            return accept.contains(value);
        };
        // greyscale
        if (m_header.is_greyscale() &&
            !is_value_in_set(m_header.bit_depth, { 1, 2, 4, 8, 16 })) {
            throw error::invalid_header_chunk("Each pixel bit depth in grayscale sample cannot be " + std::to_string(m_header.bit_depth));
        }
        // RGB
        else if (m_header.is_rgb() && 
                !is_value_in_set(m_header.bit_depth, { 8, 16 })) {
            throw error::invalid_header_chunk("Each pixel bit depth in RGB sample cannot be " + std::to_string(m_header.bit_depth));
        }
        // Pallete table
        else if (m_header.is_pallete_indexed() && 
                !is_value_in_set(m_header.bit_depth, { 1, 2, 4, 8 })) {
            throw error::invalid_header_chunk("Each pixel bit depth in pallete table cannot be " + std::to_string(m_header.bit_depth));
        }
        // greyscale with alpha
        else if (m_header.is_greyscale_with_alpha() && 
                !is_value_in_set(m_header.bit_depth, { 8, 16 })) {
            throw error::invalid_header_chunk("Each pixel bit depth in grayscale sample with alpha channel cannot be " + std::to_string(m_header.bit_depth));
        }
        // RGB with alpha
        else if (m_header.is_rgb_with_alpha() && 
                !is_value_in_set(m_header.bit_depth, { 8, 16 })) {
            throw error::invalid_header_chunk("Each pixel bit depth in RGB sample with alpha channel cannot be " + std::to_string(m_header.bit_depth));
        }

        // TODO: Validate other fields
    }

    void PNGDecoder::read_pallete() {
        // TODO: check for multiple pallete chunks and for other errors
        bool found = false;
        int index = 0;

        for (auto& chunk : m_chunks) {
            if (chunk.get_type() == Chunk::ChunkType::PALETTE) {
                found = true;
                break;
            }

            index++;
        }

        if (!found) {
            throw error::invalid_palette_chunk("Pallete chunk not found");
        }

        Chunk& pallete_chunk = m_chunks[index];

        if (pallete_chunk.get_data().size() % 3 != 0) {
            throw error::invalid_palette_chunk("Pallete chunk data length must be divisible by 3");
        }
        
        size_t length = pallete_chunk.get_data().size() / 3;
        m_pallete.entries.resize(length);

        utils::read_data_as_host_endian(
            pallete_chunk.get_data_bytes(),
            m_pallete.entries.data(),
            pallete_chunk.get_length(),
            "Cannot read pallete chunk data"
        );

        // std::cout << m_pallete.to_string(false) << std::endl;
    }

    void PNGDecoder::validate_pallete() {
        // TODO: validation
    }

    void PNGDecoder::inflate_data_chunks() {
        // merge data chunks in a single vector
        std::vector <uint8_t> merged_chunks_data;
        auto fill_with_data = [&merged_chunks_data](const std::vector <uint8_t>& data) {
            for (uint8_t byte : data) {
                merged_chunks_data.push_back(byte);
            }
        };
        
        for (auto& chunk : m_chunks) {
            if (chunk.get_type() == Chunk::ChunkType::DATA) {
                fill_with_data(chunk.get_data());
            }
        }

        // inflate
        m_image_data = inflater::Inflater().inflate(merged_chunks_data);
        // std::cout << "Inflated data size: " << m_image_data.size() << std::endl;
    }


    std::vector <PNGDecoder::IntermediateImage> PNGDecoder::defilter() {
        if (m_header.interlace_method == 0) {
            // std::cout << "No interlace defiltering method" << std::endl;
            uint32_t pos = 0;
            IntermediateImage intermediate_image = defilter_non_interlaced(m_header.width, m_header.height, pos);
            // std::cout << intermediate_image.to_string() << std::endl;     

            return { intermediate_image };
        }
        else if (m_header.interlace_method == 1) {
            // Adam7
            // std::cout << "Adam7 defiltering method" << std::endl;
            std::vector <IntermediateImage> intermediate_images = defilter_interlaced();
            return intermediate_images;
        }

        throw error::unsupported_interlace_method("interlace_method = " + std::to_string(m_header.interlace_method));
    }

    PNGDecoder::IntermediateImage PNGDecoder::defilter_non_interlaced(uint32_t width, uint32_t height, uint32_t& current_position) {
        uint32_t bits = bits_per_pixel();
        uint64_t total_bits = width * bits;
        uint32_t bytes_per_pixel = std::max(1u, bits / 8); // bytes per pixel
        
        // scanline data length
        uint32_t length = (total_bits / 8) + (total_bits % 8 != 0);

        // std::cout << "scanline lengths are: " << length << std::endl;

        Scanline previous_defiltered_scanline(0, length);
        Scanline current_scanline(0, length);

        IntermediateImage defiltered_data;
        
        defiltered_data.width = width;
        defiltered_data.height = height;

        for (size_t scanlines_read = 0; scanlines_read < height; ++scanlines_read) {
            // scanline filter_type
            utils::read_data_as_host_endian(
                m_image_data.data() + current_position,
                &current_scanline.filter_type,
                sizeof(current_scanline.filter_type),
                "Cannot read scanline filter type at index " + std::to_string(current_position)
            );

            current_position += sizeof(current_scanline.filter_type);

            // scanline data
            std::memcpy(
                current_scanline.data.data(),
                m_image_data.data() + current_position,
                length
            );

            current_position += length;

            // std::cout << "current scanline filter type is: " << static_cast<int> (current_scanline.filter_type) << std::endl;

            // apply filters
            auto defilter = Defilter::create_defilter(current_scanline.filter_type);
            defilter->apply(current_scanline, previous_defiltered_scanline, bytes_per_pixel);

            defiltered_data.data.push_back(
                std::vector<uint8_t>()
            );

            for (uint8_t byte : current_scanline.data) {
                defiltered_data.data.back().push_back(byte);
            }
            previous_defiltered_scanline = current_scanline;
        }

        // std::cout << "Current position at the end: " << current_position << std::endl; 

        return defiltered_data;
    }

    std::vector <PNGDecoder::IntermediateImage> PNGDecoder::defilter_interlaced() {
        std::vector <IntermediateImage> result;
        uint32_t position = 0;
        
        
        for (uint32_t pass = 1; pass <= 7; ++pass) {
            uint32_t w;
            uint32_t h;
            set_subimage_size(pass, w, h);

            // std::cout << "Pass: " << pass << std::endl;
            IntermediateImage img = defilter_non_interlaced(w, h, position);
            // std::cout << img.to_string() << std::endl;
            
            result.push_back(img);
        }

        return result;
    }

    void PNGDecoder::set_subimage_size(uint32_t pass, uint32_t &w, uint32_t &h) {
        uint32_t W = m_header.width;
        uint32_t H = m_header.height;
        
        if (pass == 1) {
            w = W / 8 + (W % 8 > 0);
            h = H / 8 + (H % 8 > 0);
        }
        else if (pass == 2) {
            w = W / 8 + (W % 8 > 4);
            h = H / 8 + (H % 8 > 0);
        }
        else if (pass == 3) {
            w = 2 * (W / 8) + (W % 8 > 0) + (W % 8 > 4);
            h = H / 8 + (H % 8 > 4);
        }
        else if (pass == 4) {
            w = 2 * (W / 8) + (W % 8 > 2) + (W % 8 > 6);
            h = 2 * (H / 8) + (H % 8 > 0) + (H % 8 > 4);
        }
        else if (pass == 5) {
            w = 4 * (W / 8) + (W % 8 > 0) + (W % 8 > 2) + (W % 8 > 4) + (W % 8 > 6);
            h = 2 * (H / 8) + (H % 8 > 2) + (H % 8 > 6);
        }
        else if (pass == 6) {
            w = 4 * (W / 8) + (W % 8 > 1) + (W % 8 > 3) + (W % 8 > 5);
            h = 4 * (H / 8) + (H % 8 > 0) + (H % 8 > 2) + (H % 8 > 4) + (W % 8 > 6);
        }
        else if (pass == 7) {
            w = 8 * (W / 8) + (W % 8);
            h = 4 * (H / 8) + (H % 8 > 1) + (H % 8 > 3) + (H % 8 > 5);
        }
        else {
            throw ::error::invalid_arguments("PNGDecoder::set_subimage_size `pass` must be in range [1, 7], but provided: " + std::to_string(pass));
        }
    }

    uint32_t PNGDecoder::bits_per_pixel() const {
        uint32_t bits = 0;
        
        if (m_header.is_pallete_indexed()) {
            bits = m_header.bit_depth;
        }
        else if (m_header.is_greyscale()) {
            bits = m_header.bit_depth;
        }
        else if (m_header.is_greyscale_with_alpha()) {
            bits = m_header.bit_depth * 2;
        }
        else if (m_header.is_rgb()) {
            bits = m_header.bit_depth * 3;
        }
        else if (m_header.is_rgb_with_alpha()) {
            bits = m_header.bit_depth * 4;
        }

        // std::cout << "bits per pixel: " << bits << std::endl;

        return bits;
    }


    Image PNGDecoder::create_image(std::vector <IntermediateImage>& parts) {
        auto pixel_reader = PixelReader::create_pixel_reader(get_pixel_type(), m_header.bit_depth, m_pallete);
        auto bits = bits_per_pixel();
        
        if (parts.size() == 1) {
            Image result(m_header.height, m_header.width);
            IntermediateImage& image = parts[0];

            for (size_t h = 0; h < image.height; ++h) {
                for (size_t w = 0; w < image.width; ++w) {
                    std::optional <RGB> pixel = pixel_reader->get_pixel_at(
                        image.data[h],
                        w,
                        bits
                    );

                    if (pixel.has_value()) {
                        result(h, w) = pixel.value();
                    }
                }
            }

            return result;
        }
        else if (parts.size() == 7) {
            Image result(m_header.height, m_header.width);

            for (int pass = 1; pass <= 7; ++pass) {
                auto& image = parts[pass - 1];

                for (size_t h = 0; h < image.height; ++h) {
                    for (size_t w = 0; w < image.width; ++w) {
                        std::optional <RGB> pixel = pixel_reader->get_pixel_at(image.data[h], w, bits);

                        if (pixel.has_value()) {
                            size_t W;
                            size_t H;
                            set_image_pos_by_subimage(w, h, pass, W, H);
                            result(H, W) = pixel.value();
                        }
                    }
                }
            }

            return result;
        }

        throw ::error::invalid_arguments("parts size must either 1 or 7, but got " + std::to_string(parts.size()));
    }

    void PNGDecoder::set_image_pos_by_subimage(size_t w, size_t h, int pass, size_t& W, size_t& H) {
        if (pass == 1) {
            W = 8 * w;
            H = 8 * h;
        }
        else if (pass == 2) {
            W = 8 * w + 4;
            H = 8 * h;
        }
        else if (pass == 3) {
            W = 4 * w;
            H = 8 * h + 4;
        }
        else if (pass == 4) {
            W = 4 * w + 2;
            H = 4 * h;
        }
        else if (pass == 5) {
            W = 2 * w;
            H = 4 * h + 2;
        }
        else if (pass == 6) {
            W = 2 * w + 1;
            H = 2 * h;
        }
        else if (pass == 7) {
            W = w;
            H = 2 * h + 1;
        }
        else {
            throw ::error::invalid_arguments("PNGDecoder::set_image_pos_by_subimage `pass` must be in range [1, 7], but provided: " + std::to_string(pass));
        }
    }

    PixelReader::PixelType PNGDecoder::get_pixel_type() const {
        if (m_header.is_rgb()) {
            return PixelReader::PixelType::RGB;
        }
        else if (m_header.is_rgb_with_alpha()) {
            return PixelReader::PixelType::RGB_WITH_ALPHA;
        }
        else if (m_header.is_greyscale()) {
            return PixelReader::PixelType::GREYSCALE;
        }
        else if (m_header.is_greyscale_with_alpha()) {
            return PixelReader::PixelType::GRAYSCALE_WITH_ALPHA;
        }
        else if (m_header.is_pallete_indexed()) {
            return PixelReader::PixelType::PALLETE;
        }
        else {
            throw ::error::arbitrary_error("Unknown pixel type at `PNGDecoder::get_pixel_type`");
        }
    }

    std::string PNGDecoder::Header::to_string() const {
        std::stringstream ss;
        ss << "======== PNG Header ========"   << std::endl
            << "Width: "              << width << std::endl
            << "Height: "             << height << std::endl
            << "Bit depth: "          << static_cast<int>(bit_depth) << std::endl
            << "Color type: "         << static_cast<int>(color_type) << std::endl
            << "Compression method: " << static_cast<int>(compression_method) << std::endl
            << "Filter method: "      << static_cast<int>(filter_method) << std::endl
            << "Interlace method: "   << static_cast<int>(interlace_method) << std::endl;
        return ss.str();
    }

    bool PNGDecoder::Header::is_greyscale() const noexcept {
        return color_type == 0;
    }
    bool PNGDecoder::Header::is_greyscale_with_alpha() const noexcept {
        return color_type == 4;
    }
    bool PNGDecoder::Header::is_rgb() const noexcept {
        return color_type == 2;
    }
    bool PNGDecoder::Header::is_rgb_with_alpha() const noexcept {
        return color_type == 6;
    }
    bool PNGDecoder::Header::is_pallete_indexed() const noexcept {
        return color_type == 3;
    }

    std::string PNGDecoder::IntermediateImage::to_string() const {
        std::stringstream ss;
        ss << "IntermediateImage: (w: " << width << ", h: " << height << "), data length: " << data.size() << std::endl;

        return ss.str();
    }
}



