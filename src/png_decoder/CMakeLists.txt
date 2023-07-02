set(PNG_DECODER_SOURCES
    png_decoder.h png_decoder.cpp
    chunk.h chunk.cpp
    pallete.h pallete.cpp
    defilter.h defilter.cpp
    pixel_reader.h pixel_reader.cpp
    bit_reader.h bit_reader.cpp
)

add_library(png_decoder_lib STATIC ${PNG_DECODER_SOURCES})

# The following line is very practical:
# it will allow you to automatically add the correct include directories with "target_link_libraries"
target_include_directories(png_decoder_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})