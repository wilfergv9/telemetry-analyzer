#pragma once

#include <chrono>
#include <optional>

struct TelemetrySample {
    std::chrono::system_clock::time_point timestamp;
    std::optional<double> cpuUsagePercentage;
    std::optional<double> cpuTemperatureCelsius;
    std::optional<double> ramUsageMb;
    std::optional<double> ramUsagePercentage;
};
