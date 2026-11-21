cmake_minimum_required(VERSION 3.20)
project(TelemetryAnalyzer VERSION 0.1.0 LANGUAGES CXX)

# Strict C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Strict warnings for clean and safe code
add_compile_options(-Wall -Wextra -Wpedantic)

# Main executable
add_executable(telemetry_engine
    src/main.cpp
    src/core/CPUMonitor.cpp
    src/core/MemoryMonitor.cpp
    src/core/TelemetryEngine.cpp
    src/core/CsvSessionWriter.cpp
)

# Header directory (public/internal)
target_include_directories(telemetry_engine PRIVATE include)



