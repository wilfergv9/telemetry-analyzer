#include "core/CPUMonitor.hpp"
#include "core/MemoryMonitor.hpp"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>

int main() {
    CPUMonitor cpu;
    MemoryMonitor memory;

    if (!cpu.initialize()) {
        std::cerr << "[Error] Failed to initialize CPUMonitor." << std::endl;
        return 1;
    }

    std::cout << "[TelemetryAnalyzer] Sampling system metrics...\n" << std::endl;
    std::cout << std::left 
              << std::setw(8)  << "Sample"
              << std::setw(14) << "CPU (%)"
              << std::setw(14) << "CPU Temp"
              << std::setw(16) << "RAM Used (MB)"
              << std::setw(12) << "RAM (%)"
              << std::endl;
    std::cout << std::string(64, '-') << std::endl;

    for (int i = 0; i < 5; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        auto cpuUsage = cpu.getCpuUsagePercentage();
        auto cpuTemp = cpu.getPackageTemperature();
        auto memMetrics = memory.getMetrics();

        std::cout << std::left << std::setw(8) << (i + 1);

        if (cpuUsage) {
            std::cout << std::fixed << std::setprecision(1) << std::setw(14) << *cpuUsage;
        } else {
            std::cout << std::setw(14) << "N/A";
        }

        if (cpuTemp) {
            std::cout << std::fixed << std::setprecision(1) << std::setw(14) 
                      << (std::to_string(static_cast<int>(*cpuTemp)) + " °C");
        } else {
            std::cout << std::setw(14) << "N/A";
        }

        if (memMetrics) {
            double usedMb = static_cast<double>(memMetrics->usedRamKb) / 1024.0;
            std::cout << std::fixed << std::setprecision(0) << std::setw(16) << usedMb
                      << std::fixed << std::setprecision(1) << std::setw(12) << memMetrics->ramUsagePercentage;
        } else {
            std::cout << std::setw(16) << "N/A" << std::setw(12) << "N/A";
        }

        std::cout << std::endl;
    }

    return 0;
}