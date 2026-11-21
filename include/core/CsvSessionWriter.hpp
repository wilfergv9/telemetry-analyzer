#pragma once

#include <filesystem>
#include <fstream>

#include "core/TelemetryEngine.hpp"

class CsvSessionWriter {
public:
    bool open(const std::filesystem::path& filePath);
    void writeSample(const TelemetrySample& sample);
    void close();

private:
    std::ofstream file_;
};
