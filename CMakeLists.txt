cmake_minimum_required(VERSION 3.20)

project(obs-webrtc-link VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add cmake modules path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Option to build only tests (useful for CI without OBS SDK)
option(BUILD_TESTS_ONLY "Build only tests without OBS plugin" OFF)

# Find OBS Studio (skip if building tests only)
if(NOT BUILD_TESTS_ONLY)
    if(NOT DEFINED OBS_INCLUDE_SEARCH_PATH)
        set(OBS_INCLUDE_SEARCH_PATH "" CACHE PATH "OBS Studio include directory")
    endif()

    if(NOT DEFINED OBS_LIB_SEARCH_PATH)
        set(OBS_LIB_SEARCH_PATH "" CACHE PATH "OBS Studio library directory")
    endif()

    # Find libobs
    find_path(LIBOBS_INCLUDE_DIR
        NAMES obs-module.h
        HINTS
            ${OBS_INCLUDE_SEARCH_PATH}
            $ENV{OBS_INCLUDE_DIR}
            /usr/include
            /usr/local/include
        PATH_SUFFIXES
            obs
            libobs
    )

    find_library(LIBOBS_LIB
        NAMES obs libobs
        HINTS
            ${OBS_LIB_SEARCH_PATH}
            $ENV{OBS_LIB_DIR}
            /usr/lib
            /usr/local/lib
    )

    if(NOT LIBOBS_INCLUDE_DIR OR NOT LIBOBS_LIB)
        message(FATAL_ERROR "Could not find OBS Studio. Please set OBS_INCLUDE_SEARCH_PATH and OBS_LIB_SEARCH_PATH, or use -DBUILD_TESTS_ONLY=ON to build only tests")
    endif()

    message(STATUS "Found libobs include: ${LIBOBS_INCLUDE_DIR}")
    message(STATUS "Found libobs library: ${LIBOBS_LIB}")
else()
    message(STATUS "Building tests only (OBS SDK not required)")
endif()

# Dependencies
# Note: Submodules must be initialized with: git submodule update --init --recursive

# Option to build libdatachannel from source
option(BUILD_LIBDATACHANNEL "Build libdatachannel from source" ON)

if(BUILD_LIBDATACHANNEL)
    # Build libdatachannel as submodule
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/libdatachannel/CMakeLists.txt")
        message(STATUS "Building libdatachannel from submodule")

        # Configure libdatachannel options
        set(NO_EXAMPLES ON CACHE BOOL "" FORCE)
        set(NO_TESTS ON CACHE BOOL "" FORCE)
        set(NO_WEBSOCKET ON CACHE BOOL "" FORCE)

        add_subdirectory(deps/libdatachannel EXCLUDE_FROM_ALL)

        set(LibDataChannel_FOUND TRUE)
        set(LibDataChannel_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/deps/libdatachannel/include")
    else()
        message(WARNING "libdatachannel submodule not found. Run: git submodule update --init --recursive")
        set(BUILD_LIBDATACHANNEL OFF)
    endif()
endif()

if(NOT BUILD_LIBDATACHANNEL)
    # Try to find system-installed libdatachannel
    find_package(LibDataChannel REQUIRED)
    message(STATUS "Found libdatachannel: ${LibDataChannel_LIBRARIES}")
endif()

# nlohmann-json
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/nlohmann-json/CMakeLists.txt")
    message(STATUS "Using nlohmann-json from submodule")

    # Configure nlohmann-json options
    set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
    set(JSON_Install OFF CACHE BOOL "" FORCE)

    add_subdirectory(deps/nlohmann-json EXCLUDE_FROM_ALL)

    set(nlohmann_json_FOUND TRUE)
else()
    message(WARNING "nlohmann-json submodule not found. Run: git submodule update --init --recursive")

    # Try to find system-installed nlohmann-json
    find_package(nlohmann_json 3.2.0 REQUIRED)
endif()

# Google Test (for unit testing)
option(BUILD_TESTING "Build unit tests" ON)

if(BUILD_TESTING)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/googletest/CMakeLists.txt")
        message(STATUS "Using Google Test from submodule")

        # Configure Google Test options
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
        set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
        set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

        add_subdirectory(deps/googletest EXCLUDE_FROM_ALL)

        set(GTest_FOUND TRUE)
    else()
        message(WARNING "Google Test submodule not found. Run: git submodule update --init --recursive")

        # Try to find system-installed Google Test
        find_package(GTest)
        if(NOT GTest_FOUND)
            message(WARNING "Google Test not found. Tests will be disabled.")
            set(BUILD_TESTING OFF)
        endif()
    endif()
endif()

# Core library (WebRTC logic, OBS-independent)
set(CORE_SOURCES
    src/core/peer-connection.cpp
    src/core/signaling-client.cpp
    src/core/whip-client.cpp
    src/core/whep-client.cpp
)

add_library(obs-webrtc-core STATIC ${CORE_SOURCES})

target_include_directories(obs-webrtc-core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${LibDataChannel_INCLUDE_DIRS}
)

target_link_libraries(obs-webrtc-core PUBLIC
    datachannel-static
    nlohmann_json::nlohmann_json
)

# Plugin sources and build (skip if building tests only)
if(NOT BUILD_TESTS_ONLY)
    set(PLUGIN_SOURCES
        src/plugin-main.cpp
    )

    # Create plugin library
    add_library(${PROJECT_NAME} MODULE ${PLUGIN_SOURCES})

    # Include directories
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${LIBOBS_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${LibDataChannel_INCLUDE_DIRS}
    )

    # Define plugin version
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        PLUGIN_VERSION="${PROJECT_VERSION}"
    )

    # Link libraries
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${LIBOBS_LIB}
        obs-webrtc-core
    )

    # Set output name
    set_target_properties(${PROJECT_NAME} PROPERTIES
        PREFIX ""
        OUTPUT_NAME ${PROJECT_NAME}
    )

    # Platform-specific settings
    if(WIN32)
        # Windows-specific settings
        target_compile_definitions(${PROJECT_NAME} PRIVATE
            _CRT_SECURE_NO_WARNINGS
        )
    elseif(APPLE)
        # macOS-specific settings
        set_target_properties(${PROJECT_NAME} PROPERTIES
            BUNDLE TRUE
            BUNDLE_EXTENSION "so"
        )
    elseif(UNIX)
        # Linux-specific settings
        target_compile_options(${PROJECT_NAME} PRIVATE
            -fPIC
        )
    endif()
endif()

# Install plugin (skip if building tests only)
if(NOT BUILD_TESTS_ONLY)
    if(WIN32)
        install(TARGETS ${PROJECT_NAME}
            RUNTIME DESTINATION obs-plugins/64bit
        )
        install(DIRECTORY data/
            DESTINATION data/obs-plugins/${PROJECT_NAME}
        )
    elseif(APPLE)
        install(TARGETS ${PROJECT_NAME}
            LIBRARY DESTINATION obs-plugins
        )
        install(DIRECTORY data/
            DESTINATION data/obs-plugins/${PROJECT_NAME}
        )
    else()
        install(TARGETS ${PROJECT_NAME}
            LIBRARY DESTINATION lib/obs-plugins
        )
        install(DIRECTORY data/
            DESTINATION share/obs/obs-plugins/${PROJECT_NAME}
        )
    endif()
endif()

# Enable testing
enable_testing()

# Add tests subdirectory
if(BUILD_TESTING AND GTest_FOUND)
    add_subdirectory(tests)
endif()

if(BUILD_TESTS_ONLY)
    message(STATUS "OBS WebRTC Link tests configuration complete (plugin build skipped)")
else()
    message(STATUS "OBS WebRTC Link plugin configuration complete")
endif()
