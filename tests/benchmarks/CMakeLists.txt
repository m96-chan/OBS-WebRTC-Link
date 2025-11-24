# Benchmarks for OBS WebRTC Link plugin using Google Benchmark

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${LibDataChannel_INCLUDE_DIRS}
)

# Helper function to add a benchmark
function(add_webrtc_benchmark benchmark_name)
    add_executable(${benchmark_name} ${ARGN})

    target_link_libraries(${benchmark_name} PRIVATE
        benchmark::benchmark
        benchmark::benchmark_main
        obs-webrtc-core
    )

    # Add benchmark to CTest with benchmark label
    add_test(NAME ${benchmark_name} COMMAND ${benchmark_name} --benchmark_min_time=0.1)
    set_tests_properties(${benchmark_name} PROPERTIES LABELS "benchmark")
endfunction()

# WHIP connection establishment benchmark
add_webrtc_benchmark(whip_connection_benchmark
    whip_connection_benchmark.cpp
)

# WHEP connection establishment benchmark
add_webrtc_benchmark(whep_connection_benchmark
    whep_connection_benchmark.cpp
)

# P2P connection establishment benchmark
add_webrtc_benchmark(p2p_connection_benchmark
    p2p_connection_benchmark.cpp
)

# Media encoding/decoding throughput benchmark
add_webrtc_benchmark(media_throughput_benchmark
    media_throughput_benchmark.cpp
)

# Concurrent connections scalability benchmark
add_webrtc_benchmark(scalability_benchmark
    scalability_benchmark.cpp
)
