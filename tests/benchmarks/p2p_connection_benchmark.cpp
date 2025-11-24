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
        P2PConfig config;
        config.stunServers = {"stun:stun.l.google.com:19302"};
        config.role = PeerRole::Offerer;

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
        P2PConfig config;
        config.role = PeerRole::Offerer;

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
        P2PConfig config;
        config.role = PeerRole::Answerer;
        config.stunServers = {"stun:stun.l.google.com:19302"};
        config.turnServers = {
            "turn:turn1.example.com:3478",
            "turn:turn2.example.com:3478"
        };
        config.turnUsername = "testuser";
        config.turnPassword = "testpassword";

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
            P2PConfig config;
            config.stunServers = {"stun:stun.l.google.com:19302"};
            config.role = (i % 2 == 0) ? PeerRole::Offerer : PeerRole::Answerer;

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
    P2PConfig original;
    original.role = PeerRole::Offerer;
    original.stunServers = {
        "stun:stun1.l.google.com:19302",
        "stun:stun2.l.google.com:19302",
        "stun:stun3.l.google.com:19302"
    };
    original.turnServers = {
        "turn:turn1.example.com:3478",
        "turn:turn2.example.com:3478"
    };
    original.turnUsername = "test-user-with-long-username";
    original.turnPassword = "test-password-with-long-password";

    for (auto _ : state) {
        P2PConfig copy = original;
        benchmark::DoNotOptimize(copy);
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_P2PConfigCopy);

// Benchmark role-based configuration (Offerer vs Answerer)
static void BM_P2PRoleConfiguration(benchmark::State& state) {
    const PeerRole role = (state.range(0) == 0) ? PeerRole::Offerer : PeerRole::Answerer;

    for (auto _ : state) {
        P2PConfig config;
        config.role = role;
        config.stunServers = {"stun:stun.l.google.com:19302"};

        benchmark::DoNotOptimize(config);
        P2PConnection connection(config);
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations());
    state.SetLabel(role == PeerRole::Offerer ? "Offerer" : "Answerer");
}
BENCHMARK(BM_P2PRoleConfiguration)->Arg(0)->Arg(1);
