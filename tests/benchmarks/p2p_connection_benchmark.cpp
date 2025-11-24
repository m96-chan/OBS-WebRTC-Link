/**
 * @file p2p_connection_benchmark.cpp
 * @brief Benchmark for P2P connection establishment
 */

#include <benchmark/benchmark.h>
#include "core/p2p-connection.hpp"

using namespace obswebrtc::core;

// Benchmark P2P connection creation
static void BM_P2PConnectionCreation(benchmark::State& state) {
    for (auto _ : state) {
        P2PConnectionConfig config;
        config.stunServers = {"stun:stun.l.google.com:19302"};

        benchmark::DoNotOptimize(config);
        P2PConnection connection(config);
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_P2PConnectionCreation);

// Benchmark P2P configuration with multiple STUN servers
static void BM_P2PConfigWithMultipleSTUN(benchmark::State& state) {
    const int num_stun_servers = state.range(0);

    for (auto _ : state) {
        P2PConnectionConfig config;

        for (int i = 0; i < num_stun_servers; ++i) {
            config.stunServers.push_back("stun:stun" + std::to_string(i) + ".l.google.com:19302");
        }

        benchmark::DoNotOptimize(config);
        P2PConnection connection(config);
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_P2PConfigWithMultipleSTUN)->Range(1, 8);

// Benchmark P2P configuration with TURN servers
static void BM_P2PConfigWithTURN(benchmark::State& state) {
    for (auto _ : state) {
        P2PConnectionConfig config;
        config.stunServers = {"stun:stun.l.google.com:19302"};
        config.turnServers = {
            {"turn:turn1.example.com:3478", "testuser", "testpassword"},
            {"turn:turn2.example.com:3478", "testuser", "testpassword"}
        };

        benchmark::DoNotOptimize(config);
        P2PConnection connection(config);
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_P2PConfigWithTURN);

// Benchmark multiple P2P connection instances
static void BM_MultipleP2PConnections(benchmark::State& state) {
    const int num_connections = state.range(0);

    for (auto _ : state) {
        std::vector<std::unique_ptr<P2PConnection>> connections;
        connections.reserve(num_connections);

        for (int i = 0; i < num_connections; ++i) {
            P2PConnectionConfig config;
            config.stunServers = {"stun:stun.l.google.com:19302"};

            connections.push_back(std::make_unique<P2PConnection>(config));
        }

        benchmark::DoNotOptimize(connections);
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations() * num_connections);
}
BENCHMARK(BM_MultipleP2PConnections)->Range(2, 32)->Unit(benchmark::kMicrosecond);

// Benchmark P2P configuration copy
static void BM_P2PConfigCopy(benchmark::State& state) {
    P2PConnectionConfig original;
    original.stunServers = {
        "stun:stun1.l.google.com:19302",
        "stun:stun2.l.google.com:19302",
        "stun:stun3.l.google.com:19302"
    };
    original.turnServers = {
        {"turn:turn1.example.com:3478", "test-user-with-long-username", "test-password-with-long-password"},
        {"turn:turn2.example.com:3478", "test-user-with-long-username", "test-password-with-long-password"}
    };

    for (auto _ : state) {
        P2PConnectionConfig copy = original;
        benchmark::DoNotOptimize(copy);
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_P2PConfigCopy);

// Benchmark P2P connection with different configurations
static void BM_P2PConnectionConfiguration(benchmark::State& state) {
    const int config_type = state.range(0);

    for (auto _ : state) {
        P2PConnectionConfig config;

        // Different configuration types
        if (config_type == 0) {
            // Basic STUN only
            config.stunServers = {"stun:stun.l.google.com:19302"};
        } else {
            // STUN + TURN
            config.stunServers = {"stun:stun.l.google.com:19302"};
            config.turnServers = {{"turn:turn.example.com:3478", "user", "pass"}};
        }

        benchmark::DoNotOptimize(config);
        P2PConnection connection(config);
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations());
    state.SetLabel(config_type == 0 ? "STUN" : "STUN+TURN");
}
BENCHMARK(BM_P2PConnectionConfiguration)->Arg(0)->Arg(1);
