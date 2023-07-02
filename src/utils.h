#pragma once

// stl includes
#include <netinet/in.h>
#include <iostream>
#include <string>
#include <cstring>

// custom includes
#include "errors.h"


namespace utils {

    // converts the unsigned integer netlong from network byte order to host byte order.
    inline uint64_t convert_from_big_endian_to_host(uint64_t x) {
        static bool is_same_host_and_network_byte_order = (1 == ntohl(1)); 

        return (
            is_same_host_and_network_byte_order 
            ? x
            : ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32)
        );
    }

    inline uint32_t convert_from_big_endian_to_host(uint32_t x) {
        static bool is_same_host_and_network_byte_order = (1 == ntohl(1)); 

        return (
            is_same_host_and_network_byte_order 
            ? x 
            : ntohl(x)
        );
    }

    inline uint32_t convert_from_big_endian_to_host(uint16_t x) {
        static bool is_same_host_and_network_byte_order = (1 == ntohl(1)); 

        return (
            is_same_host_and_network_byte_order 
            ? x
            : ntohs(x)
        );
    }
    
    
    // read bytes from stream as big-endian and convert the value from big-endian to the endianess of the host machine
    // only supports uint64_t, uint32_t, uint16_t
    template <class T>
    inline bool read_stream_as_big_endian_and_convert_to_host_endianess(
        std::istream& stream,
        T* destination,
        size_t bytes_count,
        std::string on_throw_msg
    ) {
        if (!stream.read(reinterpret_cast<char*> (destination), bytes_count)) {
            if (stream.eof() && !stream.bad()) {
                return false;
            }
            throw png_decoder::error::unable_to_read_from_stream(on_throw_msg);
        }

        *destination = convert_from_big_endian_to_host(*destination);
        return true;
    }


    // read bytes from pointer as big-endian and convert the value from big-endian to the endianess of the host machine
    template <class T>
    inline void read_data_as_big_endian_and_convert_to_host_endianess(
        char* source,
        T* destination,
        size_t bytes_count,
        std::string on_throw_msg
    ) {
        if (!std::memcpy(reinterpret_cast<char*>(destination), source, bytes_count)) {
            throw png_decoder::error::unable_to_read_from_source(on_throw_msg);
        }

        *destination = convert_from_big_endian_to_host(*destination);
    }

    // read bytes from pointer as big-endian and convert the value from big-endian to the endianess of the host machine
    template <class T>
    inline void read_data_as_big_endian_and_convert_to_host_endianess(
        unsigned char* source,
        T* destination,
        size_t bytes_count,
        std::string on_throw_msg
    ) {
        if (!std::memcpy(reinterpret_cast<unsigned char*>(destination), source, bytes_count)) {
            throw png_decoder::error::unable_to_read_from_source(on_throw_msg);
        }

        *destination = convert_from_big_endian_to_host(*destination);
    }

    // reads bytes from stream, throws exception with the specified message
    template <class T>
    inline bool read_as_host_endian(
        std::istream& stream,
        T* destination,
        size_t bytes_count,
        std::string on_throw_msg
    ) {
        if (!stream.read(reinterpret_cast<char*> (destination), bytes_count)) {
            if (stream.eof() && !stream.bad()) {
                return false;
            }
            throw png_decoder::error::unable_to_read_from_stream(on_throw_msg);
        }

        return true;
    }

    // reads bytes from pointer, throws exception with the specified message
    template <class T>
    inline void read_data_as_host_endian(
        char* source,
        T* destination,
        size_t bytes_count,
        std::string on_throw_msg
    ) {
        if (!std::memcpy(reinterpret_cast<char*>(destination), source, bytes_count)) {
            throw png_decoder::error::unable_to_read_from_source(on_throw_msg);
        }
    }

    // reads bytes from pointer, throws exception with the specified message
    template <class T>
    inline void read_data_as_host_endian(
        unsigned char* source,
        T* destination,
        size_t bytes_count,
        std::string on_throw_msg
    ) {
        if (!std::memcpy(reinterpret_cast<unsigned char*>(destination), source, bytes_count)) {
            throw png_decoder::error::unable_to_read_from_source(on_throw_msg);
        }
    }

    // return char by hex value [0, 15] -> [0, f]
    inline char get_char_from_hex(int hex) {
        if (hex >= 0 && hex < 10) {
            return '0' + hex;
        }
        else if (hex >= 10 && hex < 16) {
            return 'a' + hex - 10;
        }

        throw error::invalid_arguments("argument must be in range [0, 15]");
    }

} // namespace utils