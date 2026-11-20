#include "core/TelemetryEngine.hpp"
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>

std::string formatOptional(const std::optional<double>& value,
                           int decimals, const std::string& unit) {
    if (!value) return "N/D";
    std::ostringstream out;
    out << std::fixed << std::setprecision(decimals) << *value << unit;
    return out.str();
}

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
                  << std::setw(14) << formatOptional(s.cpuUsagePercentage, 1, "")
                  << std::setw(14) << formatOptional(s.cpuTemperatureCelsius, 1, " °C")
                  << std::setw(16) << formatOptional(s.ramUsageMb, 0, "")
                  << std::setw(12) << formatOptional(s.ramUsagePercentage, 1, "")
                  << std::endl;
    }

    return 0;
}