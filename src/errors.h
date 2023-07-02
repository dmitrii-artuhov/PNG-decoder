#pragma once

// stl includes
#include <exception>
#include <stdexcept>

// build error message with filename and line number
static inline std::string build_error_message(const std::string& filename, int line_number, const std::string& msg) {
    return "Exception at " + filename + ":" + std::to_string(line_number) + ": " + msg; 
}

namespace error {
    
    struct unable_to_open_file : std::runtime_error {
        explicit unable_to_open_file(std::string filename) : std::runtime_error("Unable to open file at '" + filename + "'") {}
    };

    struct invalid_arguments : std::runtime_error {
        explicit invalid_arguments(std::string msg) : std::runtime_error("Invalid argument: '" + msg + "'") {}
    };

    struct arbitrary_error : std::runtime_error {
        explicit arbitrary_error(std::string msg) : std::runtime_error("Error: '" + msg + "'") {}
    };

} // namespace error

namespace png_decoder::error {

    struct unable_to_read_from_stream : std::runtime_error {
        explicit unable_to_read_from_stream(std::string msg) : std::runtime_error("Unable ro read from stream: '" + msg + "'") {}
    };

    struct unable_to_read_from_source : std::runtime_error {
        explicit unable_to_read_from_source(std::string msg) : std::runtime_error("Unable ro read from source: '" + msg + "'") {}
    };

    struct invalid_png_signature : std::runtime_error {
        explicit invalid_png_signature() : std::runtime_error("Invalid PNG signature") {}
    };

    struct invalid_header_chunk : std::runtime_error {
        explicit invalid_header_chunk(std::string msg) : std::runtime_error("Invalid header (IHDR) chunk: '" + msg + "'") {}
    };

    struct invalid_palette_chunk : std::runtime_error {
        explicit invalid_palette_chunk(std::string msg) : std::runtime_error("Invalid palette (PLTE) chunk: '" + msg + "'") {}
    };

    struct invalid_chunks_order : std::runtime_error {
        explicit invalid_chunks_order(std::string msg) : std::runtime_error("Invalid chunks order: '" + msg + "'") {}
    };

    struct invalid_crc_checksum : std::runtime_error {
        explicit invalid_crc_checksum(std::string msg) : std::runtime_error("Invalid crc checksum: '" + msg + "'") {}
    };

    struct unsupported_interlace_method : std::runtime_error {
        explicit unsupported_interlace_method(std::string msg) : std::runtime_error("Unsupported interlace method provided: '" + msg + "'") {}
    };

} // namespace png_decoder::error

namespace png_decoder::inflater::error {

    struct invalid_compression_level : std::runtime_error {
        explicit invalid_compression_level() : std::runtime_error("Zlib invalid compression level") {}
    };

    struct invalid_inflate_data : std::runtime_error {
        explicit invalid_inflate_data() : std::runtime_error("Zlib invalid inflate data") {}
    };

    struct out_of_memory : std::runtime_error {
        explicit out_of_memory() : std::runtime_error("Zlib out of memory") {}
    };

    struct version_lib_mismatch : std::runtime_error {
        explicit version_lib_mismatch() : std::runtime_error("Zlib library version mismatch") {}
    };

} // namespace png_decoder::inflater::error


