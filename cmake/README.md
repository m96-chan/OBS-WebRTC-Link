# CMake Modules

This directory contains custom CMake find modules for the OBS-WebRTC-Link project.

## FindLibDataChannel.cmake

Find module for the LibDataChannel library.

### Usage

```cmake
# Add the cmake directory to the module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Find LibDataChannel (REQUIRED makes it fail if not found)
find_package(LibDataChannel REQUIRED)

# Link to your target
target_link_libraries(your_target PRIVATE LibDataChannel::LibDataChannel)
```

### With Version Requirement

```cmake
# Require minimum version
find_package(LibDataChannel 0.17.0 REQUIRED)
```

### Optional Find

```cmake
# Optional find (doesn't fail if not found)
find_package(LibDataChannel)

if(LibDataChannel_FOUND)
    message(STATUS "Using LibDataChannel ${LibDataChannel_VERSION}")
    target_link_libraries(your_target PRIVATE LibDataChannel::LibDataChannel)
else()
    message(WARNING "LibDataChannel not found, some features will be disabled")
endif()
```

### Custom Search Paths

You can help CMake find LibDataChannel by setting environment variables or cache variables:

```bash
# Via environment variable
export LIBDATACHANNEL_DIR=/path/to/libdatachannel
cmake ..

# Via CMake cache variable
cmake .. -DLIBDATACHANNEL_DIR=/path/to/libdatachannel
```

### Search Order

The module searches for LibDataChannel in the following order:

1. **pkg-config** (if available)
2. **Git submodules** (deps/libdatachannel)
3. **Environment variable** `LIBDATACHANNEL_DIR`
4. **Standard system paths** (/usr/include, /usr/local/include, etc.)

### Result Variables

After `find_package(LibDataChannel)` is called, the following variables are set:

- `LibDataChannel_FOUND` - TRUE if library was found
- `LibDataChannel_INCLUDE_DIRS` - Include directories
- `LibDataChannel_LIBRARIES` - Library files to link
- `LibDataChannel_VERSION` - Version of the library (if detected)

### Imported Targets

- `LibDataChannel::LibDataChannel` - The LibDataChannel library target

This target automatically includes:
- Include directories
- Link libraries
- Platform-specific dependencies (ws2_32, bcrypt on Windows; Foundation, Security frameworks on macOS; Threads on Linux)

## Testing the Find Module

To test if the find module works correctly:

```bash
cd build
cmake .. -DOBS_INCLUDE_SEARCH_PATH=/path/to/obs -DOBS_LIB_SEARCH_PATH=/path/to/obs/lib
```

CMake should output:
```
-- Building libdatachannel from submodule
-- Using nlohmann-json from submodule
```

Or if using system libraries:
```
-- Found libdatachannel: /usr/local/lib/libdatachannel.so
```

## Troubleshooting

### Library Not Found

If CMake cannot find LibDataChannel:

1. **Check submodules are initialized:**
   ```bash
   git submodule update --init --recursive
   ```

2. **Set LIBDATACHANNEL_DIR:**
   ```bash
   cmake .. -DLIBDATACHANNEL_DIR=/path/to/libdatachannel
   ```

3. **Install system-wide:**
   ```bash
   # Linux
   sudo apt install libdatachannel-dev

   # macOS
   brew install libdatachannel
   ```

### Version Mismatch

If the version requirement is not met:

```
CMake Error: Could not find a configuration file for package "LibDataChannel" that is compatible with requested version "0.17.0".
```

Update the submodule or install a newer version:
```bash
git submodule update --remote deps/libdatachannel
```

## References

- [CMake find_package Documentation](https://cmake.org/cmake/help/latest/command/find_package.html)
- [CMake FindPackageHandleStandardArgs](https://cmake.org/cmake/help/latest/module/FindPackageHandleStandardArgs.html)
- [libdatachannel GitHub](https://github.com/paullouisageneau/libdatachannel)
