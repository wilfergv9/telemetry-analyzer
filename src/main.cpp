#include "core/CPUMonitor.hpp"
#include <chrono>
#include <iostream>
#include <thread>

int main() {
    CPUMonitor cpu;

    if (!cpu.initialize()) {
        std::cerr << "[Error] Failed to find 'coretemp' sensor in /sys/class/hwmon/" << std::endl;
        return 1;
    }

    std::cout << "[TelemetryAnalyzer] Sensor located. Sampling package temperature..." << std::endl;

    for (int i = 0; i < 5; ++i) {
        if (auto temp = cpu.getPackageTemperature()) {
            std::cout << "Sample " << i + 1 << ": " << *temp << " °C" << std::endl;
        } else {
            std::cerr << "Failed to read sample " << i + 1 << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    return 0;
}
