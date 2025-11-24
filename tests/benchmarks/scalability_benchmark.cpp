/**
 * @file scalability_benchmark.cpp
 * @brief Benchmark for concurrent connections scalability
 */

#include <benchmark/benchmark.h>
#include "core/whip-client.hpp"
#include "core/whep-client.hpp"
#include "core/p2p-connection.hpp"
#include <vector>
#include <memory>

using namespace obswebrtc::core;

// Benchmark concurrent WHIP client instances
static void BM_ConcurrentWHIPClients(benchmark::State& state) {
    const int num_clients = state.range(0);

    for (auto _ : state) {
        std::vector<std::unique_ptr<WHIPClient>> clients;
        clients.reserve(num_clients);

        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < num_clients; ++i) {
            WHIPConfig config;
            config.url = "https://server" + std::to_string(i % 10) + ".example.com/whip";
            config.bearerToken = "token-" + std::to_string(i);

            try {
                clients.push_back(std::make_unique<WHIPClient>(config));
            } catch (...) {
                // Ignore exceptions
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        state.SetIterationTime(elapsed.count() / 1000000.0);

        benchmark::DoNotOptimize(clients);
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations() * num_clients);
    state.SetComplexityN(num_clients);
}
BENCHMARK(BM_ConcurrentWHIPClients)
    ->RangeMultiplier(2)
    ->Range(1, 128)
    ->Unit(benchmark::kMillisecond)
    ->UseManualTime()
    ->Complexity();

// Benchmark concurrent WHEP client instances
static void BM_ConcurrentWHEPClients(benchmark::State& state) {
    const int num_clients = state.range(0);

    for (auto _ : state) {
        std::vector<std::unique_ptr<WHEPClient>> clients;
        clients.reserve(num_clients);

        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < num_clients; ++i) {
            WHEPConfig config;
            config.url = "https://server" + std::to_string(i % 10) + ".example.com/whep";
            config.bearerToken = "token-" + std::to_string(i);

            try {
                clients.push_back(std::make_unique<WHEPClient>(config));
            } catch (...) {
                // Ignore exceptions
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        state.SetIterationTime(elapsed.count() / 1000000.0);

        benchmark::DoNotOptimize(clients);
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations() * num_clients);
    state.SetComplexityN(num_clients);
}
BENCHMARK(BM_ConcurrentWHEPClients)
    ->RangeMultiplier(2)
    ->Range(1, 128)
    ->Unit(benchmark::kMillisecond)
    ->UseManualTime()
    ->Complexity();

// Benchmark concurrent P2P connections
static void BM_ConcurrentP2PConnections(benchmark::State& state) {
    const int num_connections = state.range(0);

    for (auto _ : state) {
        std::vector<std::unique_ptr<P2PConnection>> connections;
        connections.reserve(num_connections);

        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < num_connections; ++i) {
            P2PConnectionConfig config;
            config.stunServers = {"stun:stun.l.google.com:19302"};

            try {
                connections.push_back(std::make_unique<P2PConnection>(config));
            } catch (...) {
                // Ignore exceptions
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        state.SetIterationTime(elapsed.count() / 1000000.0);

        benchmark::DoNotOptimize(connections);
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations() * num_connections);
    state.SetComplexityN(num_connections);
}
BENCHMARK(BM_ConcurrentP2PConnections)
    ->RangeMultiplier(2)
    ->Range(2, 64)
    ->Unit(benchmark::kMillisecond)
    ->UseManualTime()
    ->Complexity();

// Benchmark mixed client types (WHIP + WHEP + P2P)
static void BM_MixedClientTypes(benchmark::State& state) {
    const int clients_per_type = state.range(0);

    for (auto _ : state) {
        std::vector<std::unique_ptr<WHIPClient>> whip_clients;
        std::vector<std::unique_ptr<WHEPClient>> whep_clients;
        std::vector<std::unique_ptr<P2PConnection>> p2p_connections;

        whip_clients.reserve(clients_per_type);
        whep_clients.reserve(clients_per_type);
        p2p_connections.reserve(clients_per_type);

        auto start = std::chrono::high_resolution_clock::now();

        // Create WHIP clients
        for (int i = 0; i < clients_per_type; ++i) {
            WHIPConfig config;
            config.url = "https://whip" + std::to_string(i) + ".example.com/whip";
            config.bearerToken = "whip-token-" + std::to_string(i);
            try {
                whip_clients.push_back(std::make_unique<WHIPClient>(config));
            } catch (...) {}
        }

        // Create WHEP clients
        for (int i = 0; i < clients_per_type; ++i) {
            WHEPConfig config;
            config.url = "https://whep" + std::to_string(i) + ".example.com/whep";
            config.bearerToken = "whep-token-" + std::to_string(i);
            try {
                whep_clients.push_back(std::make_unique<WHEPClient>(config));
            } catch (...) {}
        }

        // Create P2P connections
        for (int i = 0; i < clients_per_type; ++i) {
            P2PConnectionConfig config;
            config.stunServers = {"stun:stun.l.google.com:19302"};
            try {
                p2p_connections.push_back(std::make_unique<P2PConnection>(config));
            } catch (...) {}
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        state.SetIterationTime(elapsed.count() / 1000000.0);

        benchmark::DoNotOptimize(whip_clients);
        benchmark::DoNotOptimize(whep_clients);
        benchmark::DoNotOptimize(p2p_connections);
        benchmark::ClobberMemory();
    }

    const int total_clients = clients_per_type * 3;
    state.SetItemsProcessed(state.iterations() * total_clients);
    state.SetComplexityN(total_clients);
}
BENCHMARK(BM_MixedClientTypes)
    ->RangeMultiplier(2)
    ->Range(1, 32)
    ->Unit(benchmark::kMillisecond)
    ->UseManualTime()
    ->Complexity();

// Benchmark memory usage scaling
static void BM_MemoryScaling(benchmark::State& state) {
    const int num_clients = state.range(0);

    for (auto _ : state) {
        std::vector<std::unique_ptr<WHIPClient>> clients;
        clients.reserve(num_clients);

        size_t total_memory = 0;

        for (int i = 0; i < num_clients; ++i) {
            WHIPConfig config;
            config.url = "https://example" + std::to_string(i) + ".com/whip";
            config.bearerToken = "token-" + std::to_string(i);

            try {
                clients.push_back(std::make_unique<WHIPClient>(config));
                total_memory += sizeof(WHIPClient) + config.url.size() + config.bearerToken.size();
            } catch (...) {}
        }

        benchmark::DoNotOptimize(clients);
        benchmark::DoNotOptimize(total_memory);
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations() * num_clients);
}
BENCHMARK(BM_MemoryScaling)
    ->RangeMultiplier(2)
    ->Range(1, 256)
    ->Unit(benchmark::kMicrosecond);
