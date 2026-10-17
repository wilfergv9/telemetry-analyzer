#include "core/TelemetryEngine.hpp"
#include <iostream>

TelemetryEngine::TelemetryEngine(std::chrono::milliseconds sampleInterval)
    : sampleInterval_(sampleInterval) {}

TelemetryEngine::~TelemetryEngine() {
    stop();
}

bool TelemetryEngine::initialize() {
    if (!cpuMonitor_.initialize()) {
        std::cerr << "[TelemetryEngine] Failed to initialize CPUMonitor." << std::endl;
        isInitialized_ = false;
        return false;
    }

    isInitialized_ = true;
    return true;
}

void TelemetryEngine::start() {
    if (!isInitialized_ || workerThread_.joinable()) {
        return;
    }

    // Launch worker thread with automatic cancellation token
    workerThread_ = std::jthread([this](std::stop_token stopToken) {
        samplingLoop(stopToken);
    });
}

void TelemetryEngine::stop() {
    if (workerThread_.joinable()) {
        workerThread_.request_stop();
        workerThread_.join();
    }
}

bool TelemetryEngine::isRunning() const {
    return workerThread_.joinable();
}

std::vector<TelemetrySample> TelemetryEngine::getSamples() const {
    std::lock_guard<std::mutex> lock(samplesMutex_);
    return samples_;
}

void TelemetryEngine::samplingLoop(std::stop_token stopToken) {
    while (!stopToken.stop_requested()) {
        auto cpuUsage = cpuMonitor_.getCpuUsagePercentage();
        auto cpuTemp = cpuMonitor_.getPackageTemperature();
        auto memMetrics = memoryMonitor_.getMetrics();

        TelemetrySample sample;
        sample.timestamp = std::chrono::system_clock::now();
        sample.cpuUsagePercentage = cpuUsage.value_or(0.0);
        sample.cpuTemperatureCelsius = cpuTemp.value_or(0.0);

        if (memMetrics) {
            sample.ramUsageMb = static_cast<double>(memMetrics->usedRamKb) / 1024.0;
            sample.ramUsagePercentage = memMetrics->ramUsagePercentage;
        }

        {
            std::lock_guard<std::mutex> lock(samplesMutex_);
            samples_.push_back(sample);
        }

        // Sleep with periodic stop check
        auto startSleep = std::chrono::steady_clock::now();
        while (!stopToken.stop_requested() &&
               (std::chrono::steady_clock::now() - startSleep) < sampleInterval_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
}