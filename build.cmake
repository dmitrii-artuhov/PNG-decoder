add_subdirectory(src/png_decoder)
add_subdirectory(src/crc_calculator)
add_subdirectory(src/inflater)
set(PNG_STATIC png_decoder_lib crc_calculator_lib inflater_lib)
