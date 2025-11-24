/**
 * @file test_helpers.cpp
 * @brief Implementation of common test helper utilities
 */

#include "test_helpers.hpp"

#include <array>
#include <ctime>
#include <iomanip>
#include <random>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <unistd.h>
#include <fstream>
#endif

namespace obswebrtc {
namespace testing {

std::string generateRandomString(int length) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 61);

    const char* chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string result(length, '0');
    for (char& c : result) {
        c = chars[dis(gen)];
    }
    return result;
}

std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::ostringstream oss;
    oss << std::put_time(std::gmtime(&time_t_now), "%Y-%m-%dT%H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count() << 'Z';

    return oss.str();
}

bool isPortAvailable(int port) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return false;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        WSACleanup();
        return false;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(port);

    bool available = (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0);

    closesocket(sock);
    WSACleanup();

    return available;
#else
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        return false;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(port);

    bool available = (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0);

    close(sock);

    return available;
#endif
}

int findAvailablePort(int startPort, int endPort) {
    for (int port = startPort; port <= endPort; ++port) {
        if (isPortAvailable(port)) {
            return port;
        }
    }
    return 0;
}

MemoryUsage getCurrentMemoryUsage() {
    MemoryUsage usage;

#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        usage.rss = pmc.WorkingSetSize;
        usage.vms = pmc.PrivateUsage;
    }
#else
    // Read /proc/self/statm on Linux
    std::ifstream statm("/proc/self/statm");
    if (statm) {
        long pageSize = sysconf(_SC_PAGESIZE);
        long size, resident, shared, text, lib, data, dt;
        statm >> size >> resident >> shared >> text >> lib >> data >> dt;

        usage.rss = resident * pageSize;
        usage.vms = size * pageSize;
        usage.shared = shared * pageSize;
        usage.text = text * pageSize;
        usage.data = data * pageSize;
    }
#endif

    return usage;
}

bool checkMemoryLeak(const MemoryUsage& before, const MemoryUsage& after,
                     size_t thresholdBytes) {
    // Check RSS increase
    if (after.rss > before.rss) {
        size_t increase = after.rss - before.rss;
        if (increase > thresholdBytes) {
            return false; // Potential leak detected
        }
    }

    return true; // No significant leak
}

CpuUsage getCurrentCpuUsage() {
    CpuUsage usage;

#ifdef _WIN32
    FILETIME creationTime, exitTime, kernelTime, userTime;
    if (GetProcessTimes(GetCurrentProcess(), &creationTime, &exitTime, &kernelTime, &userTime)) {
        ULARGE_INTEGER kt, ut;
        kt.LowPart = kernelTime.dwLowDateTime;
        kt.HighPart = kernelTime.dwHighDateTime;
        ut.LowPart = userTime.dwLowDateTime;
        ut.HighPart = userTime.dwHighDateTime;

        // Convert to seconds
        usage.systemPercent = kt.QuadPart / 10000000.0;
        usage.userPercent = ut.QuadPart / 10000000.0;
        usage.totalPercent = usage.userPercent + usage.systemPercent;
    }
#else
    struct rusage ru;
    if (getrusage(RUSAGE_SELF, &ru) == 0) {
        usage.userPercent = ru.ru_utime.tv_sec + ru.ru_utime.tv_usec / 1000000.0;
        usage.systemPercent = ru.ru_stime.tv_sec + ru.ru_stime.tv_usec / 1000000.0;
        usage.totalPercent = usage.userPercent + usage.systemPercent;
    }
#endif

    return usage;
}

} // namespace testing
} // namespace obswebrtc
