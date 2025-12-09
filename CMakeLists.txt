cmake_minimum_required(VERSION 3.20)

# Read version from VERSION file
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" PROJECT_VERSION)
string(STRIP "${PROJECT_VERSION}" PROJECT_VERSION)

project(obs-webrtc-link VERSION ${PROJECT_VERSION})

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

    # Platform-specific OBS installation paths
    if(WIN32)
        set(OBS_PROGRAM_FILES_PATHS
            "C:/Program Files/obs-studio"
            "C:/Program Files (x86)/obs-studio"
            "$ENV{ProgramFiles}/obs-studio"
            "$ENV{ProgramW6432}/obs-studio"
        )
    elseif(APPLE)
        set(OBS_MACOS_PATHS
            "/Applications/OBS.app/Contents/Resources"
            "/usr/local/opt/obs"
            "$ENV{HOME}/Library/Application Support/obs-studio"
        )
    endif()

    # Find libobs
    find_path(LIBOBS_INCLUDE_DIR
        NAMES obs-module.h
        HINTS
            ${OBS_INCLUDE_SEARCH_PATH}
            $ENV{OBS_INCLUDE_DIR}
            ${OBS_PROGRAM_FILES_PATHS}
            ${OBS_MACOS_PATHS}
            /usr/include
            /usr/local/include
        PATH_SUFFIXES
            include
            obs
            libobs
    )

    find_library(LIBOBS_LIB
        NAMES obs libobs
        HINTS
            ${OBS_LIB_SEARCH_PATH}
            $ENV{OBS_LIB_DIR}
            ${OBS_PROGRAM_FILES_PATHS}
            ${OBS_MACOS_PATHS}
            /usr/lib
            /usr/local/lib
            /usr/lib/x86_64-linux-gnu
        PATH_SUFFIXES
            lib
            bin
            bin/64bit
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

        # Fix libsrtp compiler warnings on macOS
        if(APPLE)
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-error=shorten-64-to-32")
        endif()

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

# Google Benchmark (for performance benchmarks)
option(BUILD_BENCHMARKS "Build performance benchmarks" ON)

if(BUILD_BENCHMARKS)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/benchmark/CMakeLists.txt")
        message(STATUS "Using Google Benchmark from submodule")

        # Configure Google Benchmark options
        set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
        set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
        set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
        set(BENCHMARK_DOWNLOAD_DEPENDENCIES OFF CACHE BOOL "" FORCE)

        add_subdirectory(deps/benchmark EXCLUDE_FROM_ALL)

        set(benchmark_FOUND TRUE)
    else()
        message(WARNING "Google Benchmark submodule not found. Run: git submodule update --init --recursive")

        # Try to find system-installed Google Benchmark
        find_package(benchmark)
        if(NOT benchmark_FOUND)
            message(WARNING "Google Benchmark not found. Benchmarks will be disabled.")
            set(BUILD_BENCHMARKS OFF)
        endif()
    endif()
endif()

# Core library (WebRTC logic, OBS-independent)
set(CORE_SOURCES
    src/core/peer-connection.cpp
    src/core/signaling-client.cpp
    src/core/http-client.cpp
    src/core/whip-client.cpp
    src/core/whep-client.cpp
    src/core/p2p-connection.cpp
    src/core/reconnection-manager.cpp
    src/core/audio-only-config.cpp
    src/core/network-statistics.cpp
    src/core/hardware-encoder.cpp
    src/core/connection-manager.cpp
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

# Enable position-independent code for static library
# This is required because the static library will be linked into a shared library (plugin)
set_target_properties(obs-webrtc-core PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

# Find Qt (skip if building tests only)
if(NOT BUILD_TESTS_ONLY)
    find_package(Qt6 COMPONENTS Core Gui Widgets)
    if(NOT Qt6_FOUND)
        find_package(Qt5 5.15 COMPONENTS Core Gui Widgets)
        if(NOT Qt5_FOUND)
            message(WARNING "Qt not found. Building plugin without UI components.")
            set(QT_FOUND FALSE)
        else()
            set(QT_FOUND TRUE)
            set(QT_VERSION 5)
            message(STATUS "Found Qt5: ${Qt5_VERSION}")
        endif()
    else()
        set(QT_FOUND TRUE)
        set(QT_VERSION 6)
        message(STATUS "Found Qt6: ${Qt6_VERSION}")
    endif()

    # Enable Qt MOC, UIC, and RCC
    if(QT_FOUND)
        set(CMAKE_AUTOMOC ON)
        set(CMAKE_AUTORCC ON)
        set(CMAKE_AUTOUIC ON)
    endif()
endif()

# Plugin sources and build (skip if building tests only)
if(NOT BUILD_TESTS_ONLY)
    set(PLUGIN_SOURCES
        src/plugin-main.cpp
        src/output/obs-webrtc-output.cpp
        src/output/webrtc-output.cpp
        src/source/obs-webrtc-source.cpp
        src/source/webrtc-source.cpp
    )

    # Add UI sources if Qt is available
    if(QT_FOUND)
        list(APPEND PLUGIN_SOURCES
            src/ui/settings-dialog.cpp
        )
    endif()

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

    # Link Qt libraries if available
    if(QT_FOUND)
        if(QT_VERSION EQUAL 6)
            target_link_libraries(${PROJECT_NAME} PRIVATE
                Qt6::Core
                Qt6::Gui
                Qt6::Widgets
            )
        else()
            target_link_libraries(${PROJECT_NAME} PRIVATE
                Qt5::Core
                Qt5::Gui
                Qt5::Widgets
            )
        endif()
        target_compile_definitions(${PROJECT_NAME} PRIVATE
            ENABLE_QT_UI
        )
    endif()

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
            LIBRARY DESTINATION obs-plugins/64bit
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
if((BUILD_TESTING AND GTest_FOUND) OR (BUILD_BENCHMARKS AND benchmark_FOUND))
    add_subdirectory(tests)
endif()

# Doxygen documentation
option(BUILD_DOCUMENTATION "Build API documentation with Doxygen" OFF)

if(BUILD_DOCUMENTATION)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        message(STATUS "Doxygen found: ${DOXYGEN_EXECUTABLE}")

        # Set input and output files
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

        # Configure the file (in case we want to use CMake variables in Doxyfile)
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

        # Add custom target to build documentation
        add_custom_target(docs
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM
        )

        message(STATUS "Documentation target 'docs' configured. Run 'cmake --build . --target docs' to generate.")
    else()
        message(WARNING "Doxygen not found. Documentation cannot be built.")
    endif()
endif()

# Static analysis with clang-tidy
option(ENABLE_CLANG_TIDY "Enable clang-tidy static analysis" OFF)

if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES "clang-tidy" "clang-tidy-17" "clang-tidy-16" "clang-tidy-15")
    if(CLANG_TIDY_EXE)
        message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")

        # Set clang-tidy for all targets
        set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")

        # Add custom target for running clang-tidy
        add_custom_target(clang-tidy
            COMMAND ${CLANG_TIDY_EXE}
                -p ${CMAKE_BINARY_DIR}
                ${CMAKE_SOURCE_DIR}/src/core/*.cpp
                ${CMAKE_SOURCE_DIR}/src/output/*.cpp
                ${CMAKE_SOURCE_DIR}/src/source/*.cpp
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Running clang-tidy static analysis"
            VERBATIM
        )

        message(STATUS "Static analysis target 'clang-tidy' configured. Run 'cmake --build . --target clang-tidy' to analyze.")
    else()
        message(WARNING "clang-tidy not found. Static analysis disabled.")
    endif()
endif()

# Static analysis with cppcheck
option(ENABLE_CPPCHECK "Enable cppcheck static analysis" OFF)

if(ENABLE_CPPCHECK)
    find_program(CPPCHECK_EXE NAMES "cppcheck")
    if(CPPCHECK_EXE)
        message(STATUS "cppcheck found: ${CPPCHECK_EXE}")

        # Add custom target for running cppcheck
        add_custom_target(cppcheck
            COMMAND ${CPPCHECK_EXE}
                --enable=all
                --std=c++17
                --suppressions-list=${CMAKE_SOURCE_DIR}/.cppcheck-suppressions
                --error-exitcode=1
                --inline-suppr
                --quiet
                -I ${CMAKE_SOURCE_DIR}/src
                ${CMAKE_SOURCE_DIR}/src
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Running cppcheck static analysis"
            VERBATIM
        )

        message(STATUS "Static analysis target 'cppcheck' configured. Run 'cmake --build . --target cppcheck' to analyze.")
    else()
        message(WARNING "cppcheck not found. Static analysis disabled.")
    endif()
endif()

# Combined static analysis target
if(ENABLE_CLANG_TIDY AND ENABLE_CPPCHECK AND CLANG_TIDY_EXE AND CPPCHECK_EXE)
    add_custom_target(static-analysis
        DEPENDS clang-tidy cppcheck
        COMMENT "Running all static analysis tools"
    )
    message(STATUS "Combined static analysis target 'static-analysis' configured.")
endif()

if(BUILD_TESTS_ONLY)
    message(STATUS "OBS WebRTC Link tests configuration complete (plugin build skipped)")
else()
    message(STATUS "OBS WebRTC Link plugin configuration complete")
endif()
