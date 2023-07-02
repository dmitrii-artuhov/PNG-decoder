#include "inflater.h"

// stl includes
#include <vector>
#include <cstdint>
#include <cstring>
#include <zlib.h>

// custom includes
#include "../errors.h"


namespace png_decoder::inflater {

    Inflater::Inflater() : m_stream{} {}

    Inflater::~Inflater() {
        static_cast<void>(inflateEnd(&m_stream));
    }

    std::vector<uint8_t> Inflater::inflate(const std::vector<uint8_t>& source) {
        std::vector<uint8_t> result;
        int ret = inflate_impl(source, result);
        validate_inflate_status(ret);

        return result;
    }

    int Inflater::init_stream() {
        m_stream.zalloc = Z_NULL;
        m_stream.zfree = Z_NULL;
        m_stream.opaque = Z_NULL;
        m_stream.avail_in = 0;
        m_stream.next_in = Z_NULL;
        return inflateInit(&m_stream);
    }

    int Inflater::inflate_impl(const std::vector<uint8_t>& source, std::vector<uint8_t>& dest) {
        // initialize stream
        int ret = init_stream();
        if (ret != Z_OK) {
            return ret;
        }
        
        uint32_t inflated_bytes_count;
        size_t source_current_index = 0;
        unsigned char in[CHUNK_SIZE];
        unsigned char out[CHUNK_SIZE];

        // decompress until deflate stream ends or end of file
        do {
            size_t count = read_from_vector(in, CHUNK_SIZE, source, source_current_index);
            source_current_index += count;
            m_stream.avail_in = count;

            if (m_stream.avail_in == 0) {
                break;
            }
            m_stream.next_in = in;

            // run inflate() on input until output buffer not full
            do {
                m_stream.avail_out = CHUNK_SIZE;
                m_stream.next_out = out;

                ret = ::inflate(&m_stream, Z_NO_FLUSH);

                switch (ret) {
                    case Z_NEED_DICT:
                        ret = Z_DATA_ERROR;
                    case Z_STREAM_ERROR:
                    case Z_DATA_ERROR:
                    case Z_MEM_ERROR:
                        return ret;
                }

                inflated_bytes_count = CHUNK_SIZE - m_stream.avail_out;
                insert_inflated_bytes(out, inflated_bytes_count, dest);

            } while (m_stream.avail_out == 0);

            // done when inflate() says it's done
        } while (ret != Z_STREAM_END);

        return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
    }


    void Inflater::validate_inflate_status(int ret) {
        switch (ret) {
            case Z_STREAM_ERROR:
                throw error::invalid_compression_level();
            case Z_DATA_ERROR:
                throw error::invalid_inflate_data();
            case Z_MEM_ERROR:
                throw error::out_of_memory();
            case Z_VERSION_ERROR:
                throw error::version_lib_mismatch();
        }
    }


    std::size_t Inflater::read_from_vector(unsigned char* buffer, size_t buffer_size, const std::vector<uint8_t>& source, size_t read_bytes) {
        size_t not_read_bytes_count = (read_bytes <= source.size()) ? source.size() - read_bytes : 0;
        size_t available_bytes_count = std::min(buffer_size, not_read_bytes_count);

        std::memcpy(buffer, source.data() + read_bytes, available_bytes_count);
        return available_bytes_count;
    }

    void Inflater::insert_inflated_bytes(const unsigned char* buffer, size_t have, std::vector<uint8_t>& dest) {
        dest.reserve(dest.size() + have);
        for (size_t i = 0; i < have; ++i) {
            dest.push_back(buffer[i]);
        }
    }


} // namespace deflater
