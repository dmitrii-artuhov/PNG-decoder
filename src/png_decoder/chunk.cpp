#include "chunk.h"

// stl includes
#include <sstream>
#include <cstring>
#include <memory>

// custom includes
#include "../utils.h"

namespace png_decoder {
    Chunk::Chunk(std::string type_label, std::vector <uint8_t> data, uint32_t crc) :
        m_type_label(type_label),
        m_data(std::move(data)),
        m_crc(crc)
    {
        if (m_type_label == "IHDR") {
            m_type = ChunkType::HEADER;
        }
        else if (m_type_label == "PLTE") {
            m_type = ChunkType::PALETTE;
        }
        else if (m_type_label == "IDAT") {
            m_type = ChunkType::DATA;
        }
        else if (m_type_label == "IEND") {
            m_type = ChunkType::END;
        }
        else {
            m_type = ChunkType::ANCILLARY;
        }
    }

    Chunk::ChunkType Chunk::get_type() const noexcept {
        return m_type;
    }

    std::string Chunk::get_type_label() const {
        return m_type_label;
    }

    uint32_t Chunk::get_length() const noexcept {
        return m_data.size();
    }

    uint32_t Chunk::get_crc() const noexcept {
        return m_crc;
    }

    std::string Chunk::to_string(bool should_print_data = false) const {
        std::stringstream ss; 
        ss << "length: " << m_data.size() << std::endl
           << "type: " << m_type_label << std::endl
           << "crc: " << m_crc << std::endl;
        
        if (should_print_data) {
            ss << "data: ";
            for (size_t i = 0; i < m_data.size(); ++i) {
                ss << utils::get_char_from_hex(m_data[i] / 16) << utils::get_char_from_hex(m_data[i] % 16) << ' ';
            }
            ss << std::endl;
        }

        return ss.str();
    }

    std::vector<char> Chunk::get_crc_bytes_sequence() {
        std::vector <char> sequence(m_type_label.size() + m_data.size());

        if (!m_type_label.empty()) {
            std::memcpy(sequence.data(), m_type_label.data(), m_type_label.size());
        }
        if (!m_data.empty()) {
            std::memcpy(sequence.data() + m_type_label.size(), m_data.data(), m_data.size());
        }

        return sequence;
    }

    uint32_t Chunk::get_crc_bytes_sequence_length() const {
        return m_data.size() + m_type_label.size();
    }

    char* Chunk::get_data_bytes() {
        return reinterpret_cast<char*> (m_data.data());
    }

    std::vector<uint8_t>& Chunk::get_data() {
        return m_data;
    }
} // namespace png_decoder