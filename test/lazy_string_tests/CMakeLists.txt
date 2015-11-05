include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(lazy_string_tests
        tests.cpp
        )

target_link_libraries(lazy_string_tests gtest gtest_main)
