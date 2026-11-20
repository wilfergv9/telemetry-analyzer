# Telemetry Analyzer

> **Status: early prototype.** Not usable as a product yet. This repository is under active development and its design is still changing.

Telemetry Analyzer is an open-source tool for Linux that aims to record what happens on your machine while you play a game, and to help you analyze it afterwards: hardware behavior, performance, errors and crashes, kept as one history per game session.

The long-term goal is to turn every play session into a record you can explore, compare and diagnose later, for example:

- Did my frame times get worse after switching Proton versions?
- What was the system doing right before the game crashed?

## What works today

- A background sampling engine (C++20, `std::jthread`) that collects samples at a fixed interval.
- **CPU usage**, computed from `/proc/stat`.
- **CPU package temperature**, discovered dynamically through `hwmon` (the `coretemp` sensor, Intel only for now). If the sensor is missing, the engine keeps running and reports the temperature as `N/D`.
- **RAM usage** (and swap, parsed but not stored yet), read from `/proc/meminfo`.
- A demo `main` that samples for a few seconds and prints a table.

Example output (values will differ on your machine):

```text
Index   CPU (%)       CPU Temp      RAM Used (MB)   RAM (%)
----------------------------------------------------------------
1       0.0           45.0 °C       10214           43.1
2       7.0           45.0 °C       10217           43.1
3       3.6           45.0 °C       10218           43.1
```

## Known limitations

- The first CPU sample is not meaningful (it may show `0.0`).
- Temperature support is limited to the Intel `coretemp` sensor; AMD (`k10temp`) is not supported yet.
- No GPU metrics, no per-process data, no FPS or frame time yet.
- Samples are kept in memory only; nothing is saved to disk yet.
- Linux only.

## Build and run

Requirements:

- Linux
- CMake 3.20 or newer
- A C++20 compiler with `std::jthread` support (developed on Fedora; also builds with GCC 13)

```bash
git clone https://github.com/wilfergv9/telemetry-analyzer.git
cd telemetry-analyzer
cmake -S . -B build
cmake --build build
./build/telemetry_engine
```

## Roadmap

The design notes, scope and ideas live in [IDEAS.md](IDEAS.md). Short term:

- Treat CPU usage as an optional value as well (`N/D` when it cannot be computed).
- Write sessions to disk incrementally (CSV first).
- Collect per-process data (`/proc/<PID>/status`).
- Add GPU metrics.
- Later: frame time capture, post-session report, session comparison.

## Contributing

Feedback and ideas are welcome: please open an issue.

## License

A license has not been chosen yet.