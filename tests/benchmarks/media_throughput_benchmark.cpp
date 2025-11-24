/**
 * @file media_throughput_benchmark.cpp
 * @brief Benchmark for media data processing throughput
 */

#include <benchmark/benchmark.h>
#include <vector>
#include <cstring>
#include <random>

// Simulate media data encoding
static void BM_MediaDataEncoding(benchmark::State& state) {
    const size_t frame_size = state.range(0);
    std::vector<uint8_t> input_frame(frame_size);
    std::vector<uint8_t> output_frame(frame_size);

    // Fill with random data
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);

    for (auto& byte : input_frame) {
        byte = static_cast<uint8_t>(dis(gen));
    }

    for (auto _ : state) {
        // Simulate encoding (simple memory copy for benchmark)
        std::memcpy(output_frame.data(), input_frame.data(), frame_size);
        benchmark::DoNotOptimize(output_frame.data());
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * frame_size);
    state.SetItemsProcessed(state.iterations());
}
// HD (1280x720) to 4K (3840x2160) frame sizes
BENCHMARK(BM_MediaDataEncoding)->Range(1280*720*3, 3840*2160*3)->Unit(benchmark::kMicrosecond);

// Simulate media data decoding
static void BM_MediaDataDecoding(benchmark::State& state) {
    const size_t frame_size = state.range(0);
    std::vector<uint8_t> encoded_frame(frame_size);
    std::vector<uint8_t> decoded_frame(frame_size);

    // Fill with random data
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);

    for (auto& byte : encoded_frame) {
        byte = static_cast<uint8_t>(dis(gen));
    }

    for (auto _ : state) {
        // Simulate decoding (simple memory copy for benchmark)
        std::memcpy(decoded_frame.data(), encoded_frame.data(), frame_size);
        benchmark::DoNotOptimize(decoded_frame.data());
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * frame_size);
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_MediaDataDecoding)->Range(1280*720*3, 3840*2160*3)->Unit(benchmark::kMicrosecond);

// Benchmark frame buffer allocation
static void BM_FrameBufferAllocation(benchmark::State& state) {
    const size_t frame_size = state.range(0);

    for (auto _ : state) {
        std::vector<uint8_t> frame_buffer(frame_size);
        benchmark::DoNotOptimize(frame_buffer.data());
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * frame_size);
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_FrameBufferAllocation)->Range(1280*720*3, 3840*2160*3);

// Benchmark packet fragmentation (splitting large frames)
static void BM_PacketFragmentation(benchmark::State& state) {
    const size_t frame_size = state.range(0);
    const size_t packet_size = 1400; // Typical MTU size
    std::vector<uint8_t> frame(frame_size);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    for (auto& byte : frame) {
        byte = static_cast<uint8_t>(dis(gen));
    }

    for (auto _ : state) {
        std::vector<std::vector<uint8_t>> packets;
        size_t offset = 0;

        while (offset < frame_size) {
            size_t chunk_size = std::min(packet_size, frame_size - offset);
            std::vector<uint8_t> packet(frame.begin() + offset, frame.begin() + offset + chunk_size);
            packets.push_back(std::move(packet));
            offset += chunk_size;
        }

        benchmark::DoNotOptimize(packets);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * frame_size);
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_PacketFragmentation)->Range(1280*720*3, 3840*2160*3)->Unit(benchmark::kMicrosecond);

// Benchmark packet reassembly
static void BM_PacketReassembly(benchmark::State& state) {
    const size_t frame_size = state.range(0);
    const size_t packet_size = 1400;

    // Create packets
    std::vector<std::vector<uint8_t>> packets;
    size_t remaining = frame_size;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);

    while (remaining > 0) {
        size_t chunk_size = std::min(packet_size, remaining);
        std::vector<uint8_t> packet(chunk_size);
        for (auto& byte : packet) {
            byte = static_cast<uint8_t>(dis(gen));
        }
        packets.push_back(std::move(packet));
        remaining -= chunk_size;
    }

    for (auto _ : state) {
        std::vector<uint8_t> reassembled;
        reassembled.reserve(frame_size);

        for (const auto& packet : packets) {
            reassembled.insert(reassembled.end(), packet.begin(), packet.end());
        }

        benchmark::DoNotOptimize(reassembled.data());
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * frame_size);
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_PacketReassembly)->Range(1280*720*3, 3840*2160*3)->Unit(benchmark::kMicrosecond);

// Benchmark concurrent frame processing
static void BM_ConcurrentFrameProcessing(benchmark::State& state) {
    const int num_frames = state.range(0);
    const size_t frame_size = 1920 * 1080 * 3; // Full HD

    std::vector<std::vector<uint8_t>> frames(num_frames, std::vector<uint8_t>(frame_size));

    for (auto _ : state) {
        for (auto& frame : frames) {
            // Simulate processing
            std::memset(frame.data(), 0, frame_size);
            benchmark::DoNotOptimize(frame.data());
        }
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * num_frames * frame_size);
    state.SetItemsProcessed(state.iterations() * num_frames);
}
BENCHMARK(BM_ConcurrentFrameProcessing)->Range(1, 30)->Unit(benchmark::kMillisecond);
