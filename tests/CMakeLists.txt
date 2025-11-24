# Unit tests for OBS WebRTC Link plugin

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${LibDataChannel_INCLUDE_DIRS}
)

# Only add OBS include directory if building the plugin
if(LIBOBS_INCLUDE_DIR)
    include_directories(${LIBOBS_INCLUDE_DIR})
endif()

# Unit tests (only when BUILD_TESTING is ON)
if(BUILD_TESTING AND GTest_FOUND)
    # Enable testing
    include(GoogleTest)

    add_subdirectory(unit)
endif()

# Integration tests (to be added later)
# add_subdirectory(integration)

# Benchmarks (Google Benchmark)
if(BUILD_BENCHMARKS AND benchmark_FOUND)
    add_subdirectory(benchmarks)
endif()
