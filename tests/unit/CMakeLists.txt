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

# WHIPClient test executable
add_executable(whip_client_test
    whip_client_test.cpp
)

target_include_directories(whip_client_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)

target_link_libraries(whip_client_test PRIVATE
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    obs-webrtc-core
)

# Discover WHIP client tests
if(WIN32)
    gtest_add_tests(TARGET whip_client_test)
else()
    gtest_discover_tests(whip_client_test)
endif()

# WHEPClient test executable
add_executable(whep_client_test
    whep_client_test.cpp
)

target_include_directories(whep_client_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)

target_link_libraries(whep_client_test PRIVATE
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    obs-webrtc-core
)

# Discover WHEP client tests
if(WIN32)
    gtest_add_tests(TARGET whep_client_test)
else()
    gtest_discover_tests(whep_client_test)
endif()

# P2PConnection test executable
add_executable(p2p_connection_test
    p2p_connection_test.cpp
)

target_include_directories(p2p_connection_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)

target_link_libraries(p2p_connection_test PRIVATE
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    obs-webrtc-core
)

# Discover P2P connection tests
if(WIN32)
    gtest_add_tests(TARGET p2p_connection_test)
else()
    gtest_discover_tests(p2p_connection_test)
endif()

# WebRTCOutput test executable
add_executable(webrtc_output_test
    webrtc_output_test.cpp
    ../../src/output/webrtc-output.cpp
)

target_include_directories(webrtc_output_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)

target_link_libraries(webrtc_output_test PRIVATE
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    obs-webrtc-core
)

# Discover WebRTC output tests
if(WIN32)
    gtest_add_tests(TARGET webrtc_output_test)
else()
    gtest_discover_tests(webrtc_output_test)
endif()

# WebRTCSource test executable
add_executable(webrtc_source_test
    webrtc_source_test.cpp
    ../../src/source/webrtc-source.cpp
)

target_include_directories(webrtc_source_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)

target_link_libraries(webrtc_source_test PRIVATE
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    obs-webrtc-core
)

# Discover WebRTC source tests
if(WIN32)
    gtest_add_tests(TARGET webrtc_source_test)
else()
    gtest_discover_tests(webrtc_source_test)
endif()

# ReconnectionManager test executable
add_executable(reconnection_manager_test
    reconnection_manager_test.cpp
    ../../src/core/reconnection-manager.cpp
)

target_include_directories(reconnection_manager_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)

target_link_libraries(reconnection_manager_test PRIVATE
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
)

# Discover Reconnection Manager tests
if(WIN32)
    gtest_add_tests(TARGET reconnection_manager_test)
else()
    gtest_discover_tests(reconnection_manager_test)
endif()

# Settings Dialog test executable (requires Qt)
if(QT_FOUND)
    add_executable(settings_dialog_test
        settings_dialog_test.cpp
        ../../src/ui/settings-dialog.cpp
    )

    target_include_directories(settings_dialog_test PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src
    )

    if(QT_VERSION EQUAL 6)
        target_link_libraries(settings_dialog_test PRIVATE
            GTest::gtest
            GTest::gtest_main
            GTest::gmock
            Qt6::Core
            Qt6::Gui
            Qt6::Widgets
        )
    else()
        target_link_libraries(settings_dialog_test PRIVATE
            GTest::gtest
            GTest::gtest_main
            GTest::gmock
            Qt5::Core
            Qt5::Gui
            Qt5::Widgets
        )
    endif()

    # Discover Settings Dialog tests
    if(WIN32)
        gtest_add_tests(TARGET settings_dialog_test)
    else()
        gtest_discover_tests(settings_dialog_test)
    endif()
endif()
