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
# On Windows, defer discovery to POST_BUILD to avoid runtime dependency issues during build
if(WIN32)
    gtest_discover_tests(sample_test
        DISCOVERY_MODE POST_BUILD
    )
else()
    gtest_discover_tests(sample_test)
endif()
