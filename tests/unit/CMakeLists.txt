# Unit tests

# Sample test executable
add_executable(sample_test
    sample_test.cpp
)

target_link_libraries(sample_test PRIVATE
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    datachannel-static
    nlohmann_json::nlohmann_json
)

# Discover tests
# On Windows, use gtest_add_tests to avoid DLL path issues during discovery
if(WIN32)
    # Manual test registration - simpler and more reliable on Windows
    gtest_add_tests(TARGET sample_test)
else()
    # Use automatic discovery on Linux/macOS
    gtest_discover_tests(sample_test)
endif()

# PeerConnection test executable
add_executable(peer_connection_test
    peer_connection_test.cpp
)

target_include_directories(peer_connection_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)

target_link_libraries(peer_connection_test PRIVATE
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    obs-webrtc-core
)

# Discover peer connection tests
if(WIN32)
    gtest_add_tests(TARGET peer_connection_test)
else()
    gtest_discover_tests(peer_connection_test)
endif()

# SignalingClient test executable
add_executable(signaling_client_test
    signaling_client_test.cpp
)

target_include_directories(signaling_client_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)

target_link_libraries(signaling_client_test PRIVATE
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    obs-webrtc-core
)

# Discover signaling client tests
if(WIN32)
    gtest_add_tests(TARGET signaling_client_test)
else()
    gtest_discover_tests(signaling_client_test)
endif()
