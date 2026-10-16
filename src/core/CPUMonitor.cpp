#include "core/CPUMonitor.hpp"
#include <fstream>

namespace fs = std::filesystem;

CPUMonitor::CPUMonitor() = default;

bool CPUMonitor::initialize() {
    auto pathOpt = findCoretempPath();
    if (!pathOpt) {
        isInitialized_ = false;
        return false;
    }

    tempInputPath_ = *pathOpt;
    isInitialized_ = true;
    return true;
}

std::optional<fs::path> CPUMonitor::findCoretempPath() {
    const fs::path hwmonBase = "/sys/class/hwmon";
    if (!fs::exists(hwmonBase)) {
        return std::nullopt;
    }

    for (const auto& entry : fs::directory_iterator(hwmonBase)) {
        if (!entry.is_directory()) {
            continue;
        }

        fs::path nameFile = entry.path() / "name";
        if (fs::exists(nameFile)) {
            std::ifstream file(nameFile);
            std::string sensorName;
            if (file >> sensorName && sensorName == "coretemp") {
                // temp1_input typically maps to Package id 0
                fs::path inputPath = entry.path() / "temp1_input";
                if (fs::exists(inputPath)) {
                    return inputPath;
                }
            }
        }
    }

    return std::nullopt;
}

std::optional<double> CPUMonitor::getPackageTemperature() {
    if (!isInitialized_) {
        return std::nullopt;
    }

    std::ifstream file(tempInputPath_);
    if (!file.is_open()) {
        return std::nullopt;
    }

    long rawValue = 0;
    if (!(file >> rawValue)) {
        return std::nullopt;
    }

    // Kernel values are in millidegrees Celsius (e.g., 35000 -> 35.0 C)
    return static_cast<double>(rawValue) / 1000.0;
}
