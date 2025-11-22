# Unit tests for OBS WebRTC Link plugin

# Enable testing
include(GoogleTest)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${LIBOBS_INCLUDE_DIR}
    ${LibDataChannel_INCLUDE_DIRS}
)

# Unit tests
add_subdirectory(unit)

# Integration tests (to be added later)
# add_subdirectory(integration)
