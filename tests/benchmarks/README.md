# OBS WebRTC Link - Performance Benchmarks

This directory contains performance benchmarks for the OBS WebRTC Link plugin using [Google Benchmark](https://github.com/google/benchmark).

## Overview

These benchmarks measure the performance characteristics of various WebRTC components:

- **WHIP Client**: Connection establishment and configuration overhead
- **WHEP Client**: Connection establishment and configuration overhead
- **P2P Connection**: Peer-to-peer connection setup with various configurations
- **Media Throughput**: Frame encoding, decoding, and packet processing
- **Scalability**: Concurrent connection handling and resource usage

## Building Benchmarks

Benchmarks are built automatically when `BUILD_BENCHMARKS` is enabled (default: ON):

```bash
cmake -B build -S . -DBUILD_TESTS_ONLY=ON -DBUILD_BENCHMARKS=ON
cmake --build build
```

## Running Benchmarks

Run all benchmarks:

```bash
ctest -C Release --output-on-failure -L benchmark
```

Run a specific benchmark:

```bash
./build/tests/benchmarks/whip_connection_benchmark
./build/tests/benchmarks/whep_connection_benchmark
./build/tests/benchmarks/p2p_connection_benchmark
./build/tests/benchmarks/media_throughput_benchmark
./build/tests/benchmarks/scalability_benchmark
```

## Benchmark Options

Google Benchmark provides several command-line options:

```bash
# Run with specific number of iterations
./benchmark --benchmark_min_time=1.0

# Filter benchmarks by name
./benchmark --benchmark_filter=BM_WHIP

# Output results in JSON format
./benchmark --benchmark_format=json --benchmark_out=results.json

# Output results in CSV format
./benchmark --benchmark_format=csv --benchmark_out=results.csv

# Display help
./benchmark --help
```

## Understanding Results

Benchmark results include:

- **Time**: Average time per iteration (ns, us, ms, or s)
- **CPU**: CPU time used
- **Iterations**: Number of times the benchmark was run
- **Items/s**: Throughput (items processed per second)
- **Bytes/s**: Data throughput (bytes processed per second)

Example output:

```
-----------------------------------------------------------------
Benchmark                        Time      CPU   Iterations
-----------------------------------------------------------------
BM_WHIPClientCreation/1        450 ns   445 ns     1568000
BM_WHIPClientCreation/8       3200 ns  3180 ns      220000
BM_MultipleWHIPClients/64     285 us   282 us       2480
```

## Benchmark Descriptions

### WHIP Connection Benchmark

Tests WHIP (WebRTC-HTTP Ingestion Protocol) client performance:

- Client creation and configuration
- URL validation overhead
- Configuration copy performance
- Multiple concurrent clients

### WHEP Connection Benchmark

Tests WHEP (WebRTC-HTTP Egress Protocol) client performance:

- Client creation and configuration
- URL validation overhead
- Configuration copy performance
- Multiple concurrent clients

### P2P Connection Benchmark

Tests peer-to-peer connection performance:

- Connection creation with various configurations
- STUN server configuration impact
- TURN server configuration overhead
- Role-based configuration (Offerer vs Answerer)

### Media Throughput Benchmark

Tests media data processing performance:

- Frame encoding and decoding (HD to 4K resolutions)
- Frame buffer allocation
- Packet fragmentation and reassembly
- Concurrent frame processing

### Scalability Benchmark

Tests system scalability with multiple concurrent connections:

- Concurrent WHIP clients (1 to 128)
- Concurrent WHEP clients (1 to 128)
- Concurrent P2P connections (2 to 64)
- Mixed client types
- Memory usage scaling
- Computational complexity analysis (O(n), O(n log n), etc.)

## CI Integration

Benchmarks are automatically run in GitHub Actions CI with the following workflow:

- Build benchmarks in Release mode
- Run each benchmark suite
- Generate JSON reports
- Compare results against baseline (if available)

## Performance Regression Detection

To detect performance regressions:

1. Run benchmarks on your branch:
   ```bash
   ./benchmark --benchmark_format=json --benchmark_out=new_results.json
   ```

2. Compare with baseline:
   ```bash
   git checkout main
   ./benchmark --benchmark_format=json --benchmark_out=baseline.json
   git checkout your-branch
   ./compare.py benchmarks baseline.json new_results.json
   ```

## Best Practices

1. **Consistent Environment**: Run benchmarks on the same hardware and OS
2. **Release Build**: Always benchmark in Release mode with optimizations
3. **Stable System**: Close unnecessary applications to reduce noise
4. **Multiple Runs**: Run benchmarks multiple times to ensure consistency
5. **Baseline Comparison**: Compare results against a known baseline

## Interpreting Complexity

Some benchmarks include computational complexity analysis:

- **O(1)**: Constant time (ideal)
- **O(n)**: Linear time (scales linearly with input)
- **O(n log n)**: Log-linear time
- **O(n²)**: Quadratic time (may indicate scalability issues)

Example:

```
BM_ConcurrentWHIPClients_BigO        1.23 N      1.23 N
```

This indicates linear O(n) scaling, meaning doubling the number of clients roughly doubles the time.

## Troubleshooting

### Benchmarks Take Too Long

Reduce minimum time:

```bash
./benchmark --benchmark_min_time=0.1
```

### Inconsistent Results

Increase minimum time for more stable results:

```bash
./benchmark --benchmark_min_time=5.0
```

### Build Errors

Ensure Google Benchmark submodule is initialized:

```bash
git submodule update --init --recursive
```

## Contributing

When adding new benchmarks:

1. Follow the naming convention: `BM_<Component><Action>`
2. Use appropriate units (ns, us, ms, s)
3. Set `SetItemsProcessed` for throughput metrics
4. Set `SetBytesProcessed` for data throughput metrics
5. Add documentation to this README

## References

- [Google Benchmark Documentation](https://github.com/google/benchmark/blob/main/docs/user_guide.md)
- [WHIP Protocol Specification](https://datatracker.ietf.org/doc/html/draft-ietf-wish-whip)
- [WHEP Protocol Specification](https://datatracker.ietf.org/doc/html/draft-murillo-whep)
