#include "core/CsvSessionWriter.hpp"

#include <chrono>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string>
#include <system_error>

namespace {

// Returns the value as text, or an empty string when there is no value (N/D)
std::string toCsvField(const std::optional<double>& value, int decimals) {
    if (!value) return "";
    std::ostringstream out;
    out << std::fixed << std::setprecision(decimals) << *value;
    return out.str();
}

}  // namespace

bool CsvSessionWriter::open(const std::filesystem::path& filePath) {
    if (filePath.has_parent_path()) {
        std::error_code ec;
        std::filesystem::create_directories(filePath.parent_path(), ec);
        if (ec) return false;
    }

    file_.open(filePath);
    if (!file_.is_open()) return false;

    file_ << "timestamp_ms,cpu_usage_percent,cpu_temp_celsius,ram_used_mb,ram_used_percent\n";
    file_.flush();
    return true;
}

void CsvSessionWriter::writeSample(const TelemetrySample& sample) {
    if (!file_.is_open()) return;

    const auto timestampMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        sample.timestamp.time_since_epoch()).count();

    file_ << timestampMs << ','
          << toCsvField(sample.cpuUsagePercentage, 1) << ','
          << toCsvField(sample.cpuTemperatureCelsius, 1) << ','
          << toCsvField(sample.ramUsageMb, 0) << ','
          << toCsvField(sample.ramUsagePercentage, 1) << '\n';
    file_.flush();
}

void CsvSessionWriter::close() {
    if (file_.is_open()) file_.close();
}