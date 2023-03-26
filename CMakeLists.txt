cmake_minimum_required(VERSION 3.10)

project(ZipCpp
    VERSION 1.0.0
    DESCRIPTION ""
    HOMEPAGE_URL ""
    LANGUAGES CXX
)

include(GNUInstallDirs)
include(CTest)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  set(IS_ZIPCPP_TOPLEVEL_PROJECT TRUE)
else()
  set(IS_ZIPCPP_TOPLEVEL_PROJECT FALSE)
endif()

option(ZIPCPP_INSTALL_LIBRARY
    "Enable installing of ZipCpp library into default locations"
    ${IS_ZIPCPP_TOPLEVEL_PROJECT}
)
option(ZIPCPP_BUILD_TESTING
    "Build and run ZipCpp tests"
    ${IS_ZIPCPP_TOPLEVEL_PROJECT}
)
option(ZIPCPP_BUILD_EXAMPLE
    "Build ZipCpp example"
    ${IS_ZIPCPP_TOPLEVEL_PROJECT}
)

add_library(ZipCpp INTERFACE)
# add alias so the project can be used with add_subdirectory
add_library(ZipCpp::ZipCpp ALIAS ZipCpp)

target_include_directories(ZipCpp
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_features(ZipCpp INTERFACE cxx_std_14)

if(BUILD_TESTING AND ZIPCPP_BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(ZIPCPP_INSTALL_LIBRARY)

    # create a target set
    install(
        TARGETS ZipCpp
        EXPORT ZipCpp_Targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    include(CMakePackageConfigHelpers)

    # create a version file
    write_basic_package_version_file(
        "ZipCppConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    # create a config file
    configure_package_config_file(
        "${PROJECT_SOURCE_DIR}/cmake/ZipCppConfig.cmake.in"
        "${PROJECT_BINARY_DIR}/ZipCppConfig.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/ZipCpp/cmake
    )

    # install the target set
    install(
        EXPORT ZipCpp_Targets
        FILE ZipCppTargets.cmake
        NAMESPACE ZipCpp::
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/ZipCpp/cmake
    )
    
    # install the config file and version file
    install(
        FILES
            "${PROJECT_BINARY_DIR}/ZipCppConfig.cmake"
            "${PROJECT_BINARY_DIR}/ZipCppConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/ZipCpp/cmake
    )
    
    # install ZipCpp headers
    install(
        DIRECTORY ${PROJECT_SOURCE_DIR}/include/ZipCpp
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

endif()