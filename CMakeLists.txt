cmake_minimum_required(VERSION 3.20)

project(obs-webrtc-link VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add cmake modules path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Find OBS Studio
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
    message(FATAL_ERROR "Could not find OBS Studio. Please set OBS_INCLUDE_SEARCH_PATH and OBS_LIB_SEARCH_PATH")
endif()

message(STATUS "Found libobs include: ${LIBOBS_INCLUDE_DIR}")
message(STATUS "Found libobs library: ${LIBOBS_LIB}")

# Plugin sources
set(PLUGIN_SOURCES
    src/plugin-main.cpp
)

# Create plugin library
add_library(${PROJECT_NAME} MODULE ${PLUGIN_SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${LIBOBS_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Define plugin version
target_compile_definitions(${PROJECT_NAME} PRIVATE
    PLUGIN_VERSION="${PROJECT_VERSION}"
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    ${LIBOBS_LIB}
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

# Install plugin
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

# Enable testing
enable_testing()

message(STATUS "OBS WebRTC Link plugin configuration complete")
