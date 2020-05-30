include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library (crcx crcx.c)
add_library (crc3x crc3x.cpp)
set_property(TARGET crc3x PROPERTY CXX_STANDARD 17)
