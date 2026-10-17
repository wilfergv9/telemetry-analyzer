cmake_minimum_required(VERSION 3.20)
project(TelemetryAnalyzer VERSION 0.1.0 LANGUAGES CXX)

# Estándar C++20 estricto
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Advertencias estrictas para código limpio y seguro
add_compile_options(-Wall -Wextra -Wpedantic)

# Ejecutable principal
add_executable(telemetry_engine
    src/main.cpp
    src/core/CPUMonitor.cpp
    src/core/MemoryMonitor.cpp
    src/core/TelemetryEngine.cpp
)

# Directorio de cabeceras públicas/internas
target_include_directories(telemetry_engine PRIVATE include)
