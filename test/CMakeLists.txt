cmake_policy(SET CMP0048 NEW)

find_package(PkgConfig)

if(PKG_CONFIG_FOUND) 

pkg_search_module(GTEST REQUIRED gtest_main)

if(GTEST_FOUND)

add_executable (crcx-test crcx-test.cpp)
add_test (NAME crcx-test COMMAND crcx-test)
target_include_directories (crcx-test PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_compile_options (crcx-test PUBLIC ${GTEST_CFLAGS})
target_link_libraries (crcx-test LINK_PUBLIC crcx ${GTEST_LDFLAGS})

add_executable (crc3x-test crc3x-test.cpp)
add_test (NAME crc3x-test COMMAND crc3x-test)
target_include_directories (crc3x-test PUBLIC ${CMAKE_SOURCE_DIR}/src)
set_property(TARGET crc3x-test PROPERTY CXX_STANDARD 17)
target_compile_options (crc3x-test PUBLIC ${GTEST_CFLAGS})
target_link_libraries (crc3x-test LINK_PUBLIC crcx ${GTEST_LDFLAGS})

endif()
endif()