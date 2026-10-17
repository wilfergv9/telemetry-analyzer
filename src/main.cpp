#include "core/CPUMonitor.hpp"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>

int main() {
    CPUMonitor cpu;

    if (!cpu.initialize()) {
        std::cerr << "[Error] Failed to initialize CPUMonitor." << std::endl;
        return 1;
    }

    std::cout << "[TelemetryAnalyzer] Sampling CPU metrics (5 iterations)...\n" << std::endl;
    std::cout << std::left << std::setw(10) << "Sample" 
              << std::setw(16) << "CPU Usage (%)" 
              << std::setw(16) << "Temperature" << std::endl;
    std::cout << std::string(42, '-') << std::endl;

    for (int i = 0; i < 5; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        auto usage = cpu.getCpuUsagePercentage();
        auto temp = cpu.getPackageTemperature();

        std::cout << std::left << std::setw(10) << (i + 1);

        if (usage) {
            std::cout << std::fixed << std::setprecision(1) << std::setw(16) << *usage;
        } else {
            std::cout << std::setw(16) << "N/A";
        }

        if (temp) {
            std::cout << std::fixed << std::setprecision(1) << *temp << " °C";
        } else {
            std::cout << "N/A";
        }

        std::cout << std::endl;
    }

    return 0;
}