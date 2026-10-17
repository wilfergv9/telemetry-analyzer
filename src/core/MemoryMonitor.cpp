#include "core/MemoryMonitor.hpp"
#include <fstream>
#include <sstream>
#include <string>

std::optional<MemoryMetrics> MemoryMonitor::getMetrics() {
    std::ifstream file("/proc/meminfo");
    if (!file.is_open()) {
        return std::nullopt;
    }

    uint64_t memTotal = 0;
    uint64_t memAvailable = 0;
    uint64_t swapTotal = 0;
    uint64_t swapFree = 0;

    int fieldsFound = 0;
    std::string line;

    while (std::getline(file, line) && fieldsFound < 4) {
        std::istringstream ss(line);
        std::string key;
        uint64_t value = 0;

        if (ss >> key >> value) {
            if (key == "MemTotal:") {
                memTotal = value;
                ++fieldsFound;
            } else if (key == "MemAvailable:") {
                memAvailable = value;
                ++fieldsFound;
            } else if (key == "SwapTotal:") {
                swapTotal = value;
                ++fieldsFound;
            } else if (key == "SwapFree:") {
                swapFree = value;
                ++fieldsFound;
            }
        }
    }

    if (memTotal == 0) {
        return std::nullopt;
    }

    MemoryMetrics metrics;
    metrics.totalRamKb = memTotal;
    metrics.availableRamKb = memAvailable;
    metrics.usedRamKb = (memTotal >= memAvailable) ? (memTotal - memAvailable) : 0;
    metrics.ramUsagePercentage = (static_cast<double>(metrics.usedRamKb) / static_cast<double>(memTotal)) * 100.0;

    metrics.totalSwapKb = swapTotal;
    metrics.freeSwapKb = swapFree;
    metrics.usedSwapKb = (swapTotal >= swapFree) ? (swapTotal - swapFree) : 0;
    metrics.swapUsagePercentage = (swapTotal > 0)
        ? (static_cast<double>(metrics.usedSwapKb) / static_cast<double>(swapTotal)) * 100.0
        : 0.0;

    return metrics;
}