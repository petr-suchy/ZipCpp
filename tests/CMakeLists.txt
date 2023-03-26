cmake_minimum_required(VERSION 3.10)

include(BoostTestHelpers.cmake)

find_package(Boost COMPONENTS unit_test_framework)

if(NOT TARGET Boost::unit_test_framework)
    message(WARNING "Boost::unit_test_framework not found, tests won't be build")
    return()
endif()

find_package(libzip)

if(NOT TARGET libzip::zip)
    message(WARNING "libzip not found, tests won't be build")
    return()
endif()

add_executable(ZipCppTests)
target_sources(ZipCppTests
    PRIVATE
        src/Main.cpp
        src/Archive/ArchiveTests.cpp
)

target_link_libraries(ZipCppTests
    PRIVATE
        libzip::zip
        ZipCpp::ZipCpp
        Boost::unit_test_framework
)

boost_discover_tests(ZipCppTests)