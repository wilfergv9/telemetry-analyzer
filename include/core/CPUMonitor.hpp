#pragma once

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

private:
    std::filesystem::path tempInputPath_;
    bool isInitialized_{false};

    std::optional<std::filesystem::path> findCoretempPath();
};
