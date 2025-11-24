/**
 * @file whep_connection_benchmark.cpp
 * @brief Benchmark for WHEP client connection establishment
 */

#include <benchmark/benchmark.h>
#include "core/whep-client.hpp"

using namespace obswebrtc::core;

// Benchmark WHEP client creation and configuration
static void BM_WHEPClientCreation(benchmark::State& state) {
    for (auto _ : state) {
        WHEPConfig config;
        config.url = "https://example.com/whep";
        config.bearerToken = "test-token-" + std::to_string(state.range(0));

        benchmark::DoNotOptimize(config);
        WHEPClient client(config);
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_WHEPClientCreation)->Range(1, 1<<10);

// Benchmark WHEP configuration validation
static void BM_WHEPConfigValidation(benchmark::State& state) {
    for (auto _ : state) {
        WHEPConfig config;
        config.url = "https://example.com/whep/endpoint";
        config.bearerToken = "bearer-token-123456789";
        config.iceServers = {
            {{"urls", "stun:stun.l.google.com:19302"}},
            {{"urls", "turn:turn.example.com:3478"}, {"username", "user"}, {"credential", "pass"}}
        };

        benchmark::DoNotOptimize(config);

        try {
            WHEPClient client(config);
            benchmark::DoNotOptimize(client);
        } catch (...) {
            // Ignore exceptions for benchmark purposes
        }

        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_WHEPConfigValidation);

// Benchmark URL validation overhead
static void BM_WHEPURLValidation(benchmark::State& state) {
    std::vector<std::string> urls;
    for (int i = 0; i < state.range(0); ++i) {
        urls.push_back("https://server" + std::to_string(i) + ".example.com/whep");
    }

    size_t index = 0;
    for (auto _ : state) {
        WHEPConfig config;
        config.url = urls[index % urls.size()];

        try {
            WHEPClient client(config);
            benchmark::DoNotOptimize(client);
        } catch (...) {
            // Ignore exceptions
        }

        ++index;
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations());
    state.SetBytesProcessed(state.iterations() * urls[0].size());
}
BENCHMARK(BM_WHEPURLValidation)->Range(1, 100);

// Benchmark multiple concurrent WHEP client instances
static void BM_MultipleWHEPClients(benchmark::State& state) {
    const int num_clients = state.range(0);

    for (auto _ : state) {
        std::vector<std::unique_ptr<WHEPClient>> clients;
        clients.reserve(num_clients);

        for (int i = 0; i < num_clients; ++i) {
            WHEPConfig config;
            config.url = "https://server" + std::to_string(i) + ".example.com/whep";
            config.bearerToken = "token-" + std::to_string(i);

            clients.push_back(std::make_unique<WHEPClient>(config));
        }

        benchmark::DoNotOptimize(clients);
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations() * num_clients);
}
BENCHMARK(BM_MultipleWHEPClients)->Range(1, 64)->Unit(benchmark::kMicrosecond);

// Benchmark configuration copy overhead
static void BM_WHEPConfigCopy(benchmark::State& state) {
    WHEPConfig original;
    original.url = "https://example.com/whep";
    original.bearerToken = "bearer-token-with-long-string-data-for-testing";
    original.iceServers = {
        {{"urls", "stun:stun1.l.google.com:19302"}},
        {{"urls", "stun:stun2.l.google.com:19302"}},
        {{"urls", "turn:turn.example.com:3478"}, {"username", "testuser"}, {"credential", "testpass"}}
    };

    for (auto _ : state) {
        WHEPConfig copy = original;
        benchmark::DoNotOptimize(copy);
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_WHEPConfigCopy);
