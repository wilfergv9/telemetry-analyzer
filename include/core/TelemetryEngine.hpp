#pragma once

#include "core/CPUMonitor.hpp"
#include "core/MemoryMonitor.hpp"

#include <chrono>
#include <mutex>
#include <thread>
#include <vector>

struct TelemetrySample {
    std::chrono::system_clock::time_point timestamp;
    double cpuUsagePercentage{0.0};
    double cpuTemperatureCelsius{0.0};
    double ramUsageMb{0.0};
    double ramUsagePercentage{0.0};
};

class TelemetryEngine {
public:
    explicit TelemetryEngine(std::chrono::milliseconds sampleInterval = std::chrono::milliseconds(500));
    ~TelemetryEngine();

    // Prevent copies to avoid accidental thread/resource duplication
    TelemetryEngine(const TelemetryEngine&) = delete;
    TelemetryEngine& operator=(const TelemetryEngine&) = delete;

    bool initialize();
    void start();
    void stop();

    bool isRunning() const;
    std::vector<TelemetrySample> getSamples() const;

private:
    void samplingLoop(std::stop_token stopToken);

    std::chrono::milliseconds sampleInterval_;
    CPUMonitor cpuMonitor_;
    MemoryMonitor memoryMonitor_;

    mutable std::mutex samplesMutex_;
    std::vector<TelemetrySample> samples_;

    std::jthread workerThread_;
    bool isInitialized_{false};
};