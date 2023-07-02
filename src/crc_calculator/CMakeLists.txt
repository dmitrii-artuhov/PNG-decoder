set(CRC_CALCULATOR_SOURCES crc_calculator.h crc_calculator.cpp)


# Find Boost libraries
find_package(Boost COMPONENTS system REQUIRED)

add_library(crc_calculator_lib STATIC ${CRC_CALCULATOR_SOURCES})
target_link_libraries(crc_calculator_lib Boost::system)

# The following line is very practical:
# it will allow you to automatically add the correct include directories with "target_link_libraries"
target_include_directories(crc_calculator_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})