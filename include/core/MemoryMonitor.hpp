#pragma once

#include <cstdint>
#include <optional>

struct MemoryMetrics {
    uint64_t totalRamKb{0};
    uint64_t availableRamKb{0};
    uint64_t usedRamKb{0};
    double ramUsagePercentage{0.0};

    uint64_t totalSwapKb{0};
    uint64_t freeSwapKb{0};
    uint64_t usedSwapKb{0};
    double swapUsagePercentage{0.0};
};

class MemoryMonitor {
public:
    MemoryMonitor() = default;

    // Reads /proc/meminfo and returns system RAM/Swap metrics
    std::optional<MemoryMetrics> getMetrics();
};