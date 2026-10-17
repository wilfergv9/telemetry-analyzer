#include "core/CPUMonitor.hpp"
#include <fstream>
#include <sstream>

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

    // Primer snapshot para tener punto de referencia inicial
    previousSnapshot_ = readCpuTimeSnapshot();

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

    return static_cast<double>(rawValue) / 1000.0;
}

std::optional<CPUMonitor::CpuTimeSnapshot> CPUMonitor::readCpuTimeSnapshot() {
    std::ifstream file("/proc/stat");
    if (!file.is_open()) {
        return std::nullopt;
    }

    std::string line;
    if (!std::getline(file, line)) {
        return std::nullopt;
    }

    std::istringstream ss(line);
    std::string cpuLabel;
    ss >> cpuLabel;

    if (cpuLabel != "cpu") {
        return std::nullopt;
    }

    uint64_t user = 0, nice = 0, system = 0, idle = 0, iowait = 0;
    uint64_t irq = 0, softirq = 0, steal = 0;

    if (!(ss >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal)) {
        return std::nullopt;
    }

    uint64_t activeTime = user + nice + system + irq + softirq + steal;
    uint64_t idleTime = idle + iowait;
    uint64_t totalTime = activeTime + idleTime;

    return CpuTimeSnapshot{activeTime, totalTime};
}

std::optional<double> CPUMonitor::getCpuUsagePercentage() {
    auto currentSnapshot = readCpuTimeSnapshot();
    if (!currentSnapshot || !previousSnapshot_) {
        previousSnapshot_ = currentSnapshot;
        return std::nullopt;
    }

    uint64_t deltaActive = currentSnapshot->activeTime - previousSnapshot_->activeTime;
    uint64_t deltaTotal = currentSnapshot->totalTime - previousSnapshot_->totalTime;

    previousSnapshot_ = currentSnapshot;

    if (deltaTotal == 0) {
        return 0.0;
    }

    return (static_cast<double>(deltaActive) / static_cast<double>(deltaTotal)) * 100.0;
}