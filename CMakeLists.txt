cmake_minimum_required(VERSION 3.14)
project(dataroute)

# Setup

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BUILD_ADDITIONAL_TARGETS OFF)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(BUILD_ADDITIONAL_TARGETS ON)
endif()

# Library

if (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Generic")
    add_library(dataroute SHARED
        src/serial.cpp
    )
    target_include_directories(dataroute PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )

else()
    # TODO
endif()

# Additional targets

if (BUILD_ADDITIONAL_TARGETS AND NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Generic")
    add_executable(example_serial example/serial.cpp)
    target_link_libraries(example_serial dataroute)
endif()


# Create install target

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/datarouteConfigVersion.cmake"
    VERSION 0.1
    COMPATIBILITY AnyNewerVersion
)

install(TARGETS dataroute
    EXPORT datarouteTargets
    LIBRARY DESTINATION lib COMPONENT Runtime
    ARCHIVE DESTINATION lib COMPONENT Development
    RUNTIME DESTINATION bin COMPONENT Runtime
    PUBLIC_HEADER DESTINATION include COMPONENT Development
    BUNDLE DESTINATION bin COMPONENT Runtime
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/datarouteConfig.cmake.in"
    "${PROJECT_BINARY_DIR}/datarouteConfig.cmake"
    INSTALL_DESTINATION lib/cmake/dataroute
)

install(EXPORT datarouteTargets DESTINATION lib/cmake/dataroute)
install(FILES "${PROJECT_BINARY_DIR}/datarouteConfigVersion.cmake"
              "${PROJECT_BINARY_DIR}/datarouteConfig.cmake"
        DESTINATION lib/cmake/dataroute)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ DESTINATION include)
