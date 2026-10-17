#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>

class CPUMonitor {
public:
    CPUMonitor();

    // Dynamically locates the hwmon path associated with 'coretemp'
    bool initialize();

    // Reads the package temperature (Package id 0) in degrees Celsius
    std::optional<double> getPackageTemperature();

    // Computes CPU utilization percentage between consecutive calls
    std::optional<double> getCpuUsagePercentage();

private:
    struct CpuTimeSnapshot {
        uint64_t activeTime{0};
        uint64_t totalTime{0};
    };

    std::filesystem::path tempInputPath_;
    bool isInitialized_{false};
    std::optional<CpuTimeSnapshot> previousSnapshot_;

    std::optional<std::filesystem::path> findCoretempPath();
    std::optional<CpuTimeSnapshot> readCpuTimeSnapshot();
};