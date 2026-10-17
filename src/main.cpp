#include "core/TelemetryEngine.hpp"
#include <iomanip>
#include <iostream>

int main() {
    TelemetryEngine engine(std::chrono::milliseconds(250)); // sample every 250ms

    if (!engine.initialize()) {
        std::cerr << "[Error] Initialization failed." << std::endl;
        return 1;
    }

    std::cout << "[Main] Starting telemetry engine in background..." << std::endl;
    engine.start();

    // Simulate game/workload execution on main thread
    std::cout << "[Main] Workload running for 3 seconds..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));

    std::cout << "[Main] Stopping engine..." << std::endl;
    engine.stop();

    auto samples = engine.getSamples();
    std::cout << "[Main] Capture finished. Total samples collected: " << samples.size() << "\n" << std::endl;

    std::cout << std::left 
              << std::setw(8)  << "Index"
              << std::setw(14) << "CPU (%)"
              << std::setw(14) << "CPU Temp"
              << std::setw(16) << "RAM Used (MB)"
              << std::setw(12) << "RAM (%)"
              << std::endl;
    std::cout << std::string(64, '-') << std::endl;

    for (size_t i = 0; i < samples.size(); ++i) {
        const auto& s = samples[i];
        std::cout << std::left << std::setw(8) << (i + 1)
                  << std::fixed << std::setprecision(1) << std::setw(14) << s.cpuUsagePercentage
                  << std::fixed << std::setprecision(1) << std::setw(14) << (std::to_string(static_cast<int>(s.cpuTemperatureCelsius)) + " °C")
                  << std::fixed << std::setprecision(0) << std::setw(16) << s.ramUsageMb
                  << std::fixed << std::setprecision(1) << std::setw(12) << s.ramUsagePercentage
                  << std::endl;
    }

    return 0;
}