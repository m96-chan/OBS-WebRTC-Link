/**
 * @file whip_connection_benchmark.cpp
 * @brief Benchmark for WHIP client connection establishment
 */

#include <benchmark/benchmark.h>
#include "core/whip-client.hpp"

using namespace obswebrtc::core;

// Benchmark WHIP client creation and configuration
static void BM_WHIPClientCreation(benchmark::State& state) {
    for (auto _ : state) {
        WHIPConfig config;
        config.url = "https://example.com/whip";
        config.bearerToken = "test-token-" + std::to_string(state.range(0));

        benchmark::DoNotOptimize(config);
        WHIPClient client(config);
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_WHIPClientCreation)->Range(1, 1<<10);

// Benchmark WHIP configuration validation
static void BM_WHIPConfigValidation(benchmark::State& state) {
    for (auto _ : state) {
        WHIPConfig config;
        config.url = "https://example.com/whip/endpoint";
        config.bearerToken = "bearer-token-123456789";
        config.iceServers = {
            {{"urls", "stun:stun.l.google.com:19302"}},
            {{"urls", "turn:turn.example.com:3478"}, {"username", "user"}, {"credential", "pass"}}
        };

        benchmark::DoNotOptimize(config);

        try {
            WHIPClient client(config);
            benchmark::DoNotOptimize(client);
        } catch (...) {
            // Ignore exceptions for benchmark purposes
        }

        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_WHIPConfigValidation);

// Benchmark URL validation overhead
static void BM_URLValidation(benchmark::State& state) {
    std::vector<std::string> urls;
    for (int i = 0; i < state.range(0); ++i) {
        urls.push_back("https://server" + std::to_string(i) + ".example.com/whip");
    }

    size_t index = 0;
    for (auto _ : state) {
        WHIPConfig config;
        config.url = urls[index % urls.size()];

        try {
            WHIPClient client(config);
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
BENCHMARK(BM_URLValidation)->Range(1, 100);

// Benchmark multiple concurrent WHIP client instances
static void BM_MultipleWHIPClients(benchmark::State& state) {
    const int num_clients = state.range(0);

    for (auto _ : state) {
        std::vector<std::unique_ptr<WHIPClient>> clients;
        clients.reserve(num_clients);

        for (int i = 0; i < num_clients; ++i) {
            WHIPConfig config;
            config.url = "https://server" + std::to_string(i) + ".example.com/whip";
            config.bearerToken = "token-" + std::to_string(i);

            clients.push_back(std::make_unique<WHIPClient>(config));
        }

        benchmark::DoNotOptimize(clients);
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations() * num_clients);
}
BENCHMARK(BM_MultipleWHIPClients)->Range(1, 64)->Unit(benchmark::kMicrosecond);

// Benchmark configuration copy overhead
static void BM_WHIPConfigCopy(benchmark::State& state) {
    WHIPConfig original;
    original.url = "https://example.com/whip";
    original.bearerToken = "bearer-token-with-long-string-data-for-testing";
    original.iceServers = {
        {{"urls", "stun:stun1.l.google.com:19302"}},
        {{"urls", "stun:stun2.l.google.com:19302"}},
        {{"urls", "turn:turn.example.com:3478"}, {"username", "testuser"}, {"credential", "testpass"}}
    };

    for (auto _ : state) {
        WHIPConfig copy = original;
        benchmark::DoNotOptimize(copy);
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_WHIPConfigCopy);
