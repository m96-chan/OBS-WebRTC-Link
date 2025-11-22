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

# Integration tests (to be added later)
# add_subdirectory(integration)
