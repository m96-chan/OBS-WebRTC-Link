#[=======================================================================[.rst:
FindLibDataChannel
------------------

Find the LibDataChannel library.

IMPORTED Targets
^^^^^^^^^^^^^^^^

This module defines the following :prop_tgt:`IMPORTED` targets:

``LibDataChannel::LibDataChannel``
  The LibDataChannel library, if found.

Result Variables
^^^^^^^^^^^^^^^^

This module will set the following variables in your project:

``LibDataChannel_FOUND``
  True if LibDataChannel is found.
``LibDataChannel_INCLUDE_DIRS``
  Include directories for LibDataChannel.
``LibDataChannel_LIBRARIES``
  Libraries to link against LibDataChannel.
``LibDataChannel_VERSION``
  The version of LibDataChannel found.

#]=======================================================================]

# First, try to find LibDataChannel using pkg-config
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_LibDataChannel QUIET libdatachannel)
endif()

# Find include directory
find_path(LibDataChannel_INCLUDE_DIR
    NAMES rtc/rtc.h rtc/rtc.hpp
    HINTS
        ${PC_LibDataChannel_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/deps/libdatachannel/include
        ${CMAKE_SOURCE_DIR}/deps/libdatachannel/include
    PATHS
        /usr/include
        /usr/local/include
        $ENV{LIBDATACHANNEL_DIR}/include
    PATH_SUFFIXES
        libdatachannel
)

# Find library
find_library(LibDataChannel_LIBRARY
    NAMES datachannel libdatachannel datachannel-static
    HINTS
        ${PC_LibDataChannel_LIBRARY_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/deps/libdatachannel/build
        ${CMAKE_SOURCE_DIR}/deps/libdatachannel/build
    PATHS
        /usr/lib
        /usr/local/lib
        $ENV{LIBDATACHANNEL_DIR}/lib
    PATH_SUFFIXES
        lib
        lib64
)

# Extract version information
if(LibDataChannel_INCLUDE_DIR AND EXISTS "${LibDataChannel_INCLUDE_DIR}/rtc/version.h")
    file(STRINGS "${LibDataChannel_INCLUDE_DIR}/rtc/version.h" version_line
         REGEX "^#define[ \t]+RTC_VERSION[ \t]+\".*\"")
    if(version_line)
        string(REGEX REPLACE "^#define[ \t]+RTC_VERSION[ \t]+\"([^\"]*)\".*" "\\1"
               LibDataChannel_VERSION "${version_line}")
    endif()
endif()

# Use pkg-config version if not found in header
if(NOT LibDataChannel_VERSION AND PC_LibDataChannel_VERSION)
    set(LibDataChannel_VERSION ${PC_LibDataChannel_VERSION})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibDataChannel
    REQUIRED_VARS
        LibDataChannel_LIBRARY
        LibDataChannel_INCLUDE_DIR
    VERSION_VAR
        LibDataChannel_VERSION
)

if(LibDataChannel_FOUND)
    set(LibDataChannel_LIBRARIES ${LibDataChannel_LIBRARY})
    set(LibDataChannel_INCLUDE_DIRS ${LibDataChannel_INCLUDE_DIR})

    # Create imported target
    if(NOT TARGET LibDataChannel::LibDataChannel)
        add_library(LibDataChannel::LibDataChannel UNKNOWN IMPORTED)
        set_target_properties(LibDataChannel::LibDataChannel PROPERTIES
            IMPORTED_LOCATION "${LibDataChannel_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${LibDataChannel_INCLUDE_DIR}"
        )

        # Add platform-specific link libraries
        if(WIN32)
            set_property(TARGET LibDataChannel::LibDataChannel APPEND PROPERTY
                INTERFACE_LINK_LIBRARIES ws2_32 bcrypt)
        elseif(APPLE)
            set_property(TARGET LibDataChannel::LibDataChannel APPEND PROPERTY
                INTERFACE_LINK_LIBRARIES "-framework Foundation" "-framework Security")
        else()
            # Linux/Unix
            find_package(Threads REQUIRED)
            set_property(TARGET LibDataChannel::LibDataChannel APPEND PROPERTY
                INTERFACE_LINK_LIBRARIES Threads::Threads)
        endif()
    endif()

    mark_as_advanced(
        LibDataChannel_INCLUDE_DIR
        LibDataChannel_LIBRARY
    )
endif()
