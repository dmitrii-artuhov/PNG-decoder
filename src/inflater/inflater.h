#pragma once

// stl includes
#include <vector>
#include <cstdint>
#include <zlib.h>

// custom includes



namespace png_decoder::inflater {

class Inflater {
public:
    Inflater();
    ~Inflater();

    std::vector<uint8_t> inflate(const std::vector<uint8_t>& source);

private:
    inline static const size_t CHUNK_SIZE = 16384;
    z_stream m_stream;


    /*
    Decompress from `source` to `dest`.
    returns
    - Z_OK on success
    - Z_MEM_ERROR if memory could not be allocated for processing
    - Z_DATA_ERROR if the deflate data is invalid or incomplete
    - Z_VERSION_ERROR if the version of zlib.h and the version of the library linked do not match.
    */
    int inflate_impl(const std::vector<uint8_t>& source, std::vector<uint8_t>& dest);

    int init_stream();
    
    void validate_inflate_status(int ret);
    
    std::size_t read_from_vector(unsigned char* buffer, size_t buffer_size, const std::vector<uint8_t>& source, size_t start);
    
    void insert_inflated_bytes(const unsigned char* buffer, size_t have, std::vector<uint8_t>& dest);
};

    

}