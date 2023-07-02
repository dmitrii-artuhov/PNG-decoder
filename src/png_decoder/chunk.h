#pragma once

// stl includes
#include <vector>
#include <fstream>
#include <cstdint>
#include <memory>

namespace png_decoder {
    // inner classes definitions
    class Chunk {
    public:
        enum class ChunkType : uint32_t {
            HEADER,
            PALETTE,
            DATA,
            END,
            ANCILLARY // helper type
        };

        Chunk(std::string type_label, std::vector <uint8_t> data, uint32_t crc);

        ChunkType get_type() const noexcept;
        std::string get_type_label() const;
        uint32_t get_length() const noexcept; // length is always `uint32_t`
        uint32_t get_crc() const noexcept;
        std::string to_string(bool should_print_data) const;
        char* get_data_bytes();
        std::vector<char> get_crc_bytes_sequence();
        uint32_t get_crc_bytes_sequence_length() const;
        std::vector<uint8_t>& get_data();

    private:
        std::string m_type_label;
        ChunkType m_type;
        std::vector <uint8_t> m_data;
        uint32_t m_crc; // cyclic redundancy check
    };
} // namespace png_decoder
