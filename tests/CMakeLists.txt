# Unit tests for OBS WebRTC Link plugin

# Enable testing
include(GoogleTest)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${LibDataChannel_INCLUDE_DIRS}
)

# Only add OBS include directory if building the plugin
if(LIBOBS_INCLUDE_DIR)
    include_directories(${LIBOBS_INCLUDE_DIR})
endif()

# Unit tests
add_subdirectory(unit)

# Integration tests (optional)
option(BUILD_INTEGRATION_TESTS "Build integration tests" OFF)
if(BUILD_INTEGRATION_TESTS AND nlohmann_json_FOUND)
    add_subdirectory(integration)
    message(STATUS "Integration tests enabled")
else()
    message(STATUS "Integration tests disabled (enable with -DBUILD_INTEGRATION_TESTS=ON)")
endif()
