set(INFLATER_SOURCES inflater.h inflater.cpp)


# Find Boost libraries
find_package(ZLIB REQUIRED)

add_library(inflater_lib STATIC ${INFLATER_SOURCES})
target_link_libraries(inflater_lib ZLIB::ZLIB)

# The following line is very practical:
# it will allow you to automatically add the correct include directories with "target_link_libraries"
target_include_directories(inflater_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})