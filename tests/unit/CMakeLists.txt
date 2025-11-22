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
