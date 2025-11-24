# Integration tests for OBS WebRTC Link plugin
#
# These tests require:
# - Docker and Docker Compose for LiveKit server
# - Network access for STUN/TURN servers
# - Sufficient resources for concurrent connections
#
# To run integration tests:
# - Build with -DBUILD_TESTING=ON
# - Ensure Docker is running
# - Run: ctest -R Integration

# Helper function to create Docker-based integration tests
function(add_integration_test test_name)
    add_executable(${test_name}
        ${ARGN}
    )

    target_include_directories(${test_name} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src
    )

    target_link_libraries(${test_name} PRIVATE
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
        obs-webrtc-core
    )

    # Mark as integration test (longer timeout)
    gtest_discover_tests(${test_name}
        PROPERTIES
            LABELS "integration"
            TIMEOUT 300  # 5 minutes for integration tests
    )
endfunction()

# Test helper library for common integration test utilities
add_library(integration-test-helpers STATIC
    helpers/livekit_docker_manager.cpp
    helpers/test_signaling_server.cpp
    helpers/test_helpers.cpp
)

target_include_directories(integration-test-helpers PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)

target_link_libraries(integration-test-helpers PUBLIC
    GTest::gtest
    GTest::gmock
    obs-webrtc-core
    nlohmann_json::nlohmann_json
)

# LiveKit integration tests (requires Docker)
add_integration_test(livekit_integration_test
    livekit_integration_test.cpp
)

target_link_libraries(livekit_integration_test PRIVATE
    integration-test-helpers
)

# P2P connection integration tests
add_integration_test(p2p_integration_test
    p2p_integration_test.cpp
)

target_link_libraries(p2p_integration_test PRIVATE
    integration-test-helpers
)

# End-to-end flow tests (WHIP publish + WHEP subscribe)
add_integration_test(e2e_flow_test
    e2e_flow_test.cpp
)

target_link_libraries(e2e_flow_test PRIVATE
    integration-test-helpers
)

# Performance tests (throughput, latency, resource usage)
add_integration_test(performance_test
    performance_test.cpp
)

target_link_libraries(performance_test PRIVATE
    integration-test-helpers
)

# Stability tests (long-duration, reconnection)
add_integration_test(stability_test
    stability_test.cpp
)

target_link_libraries(stability_test PRIVATE
    integration-test-helpers
)

# Memory leak tests (requires AddressSanitizer or Valgrind)
if(CMAKE_BUILD_TYPE MATCHES "Debug|RelWithDebInfo")
    add_integration_test(memory_leak_test
        memory_leak_test.cpp
    )

    target_link_libraries(memory_leak_test PRIVATE
        integration-test-helpers
    )

    # Enable AddressSanitizer on supported platforms
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(memory_leak_test PRIVATE -fsanitize=address -fno-omit-frame-pointer)
        target_link_options(memory_leak_test PRIVATE -fsanitize=address)
    endif()
endif()

# Copy Docker Compose configuration to build directory
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/docker/docker-compose.yml
    ${CMAKE_CURRENT_BINARY_DIR}/docker-compose.yml
    COPYONLY
)

# Copy test scripts
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/run_integration_tests.sh
    ${CMAKE_CURRENT_BINARY_DIR}/run_integration_tests.sh
    COPYONLY
)

if(WIN32)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/scripts/run_integration_tests.ps1
        ${CMAKE_CURRENT_BINARY_DIR}/run_integration_tests.ps1
        COPYONLY
    )
endif()

message(STATUS "Integration tests configured (requires Docker for LiveKit tests)")
