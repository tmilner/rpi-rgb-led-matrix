# Repository Guidelines

## Project Structure & Module Organization
- `screen-test-1.cc` is the main executable entry point for the custom display.
- `lib/` holds core implementation (rendering, GPIO, clients) with matching headers in `include/`.
- `utils/` contains standalone tools like `led-image-viewer` and `text-scroller` with their own README.
- `fonts/` and `img/` provide BDF fonts and image assets used by screens/utilities.
- `wiring.md` and `README.md` document hardware setup and LED matrix flags.

## Build, Test, and Development Commands
- Install dependencies with Conan and generate the toolchain: `conan install . --output-folder=build --build=missing -pr:h <pi-zero-w-profile> -pr:b <build-profile>`.
- Configure with CMake using the Conan toolchain: `cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake`.
- Build the target: `cmake --build build` (produces `ScreenTest`).
- For a debug/valgrind build, use the preset after a matching Conan install: `cmake --preset debug-valgrind` then `cmake --build --preset debug-valgrind`.
- Utilities can be built with `make led-image-viewer` or `make text-scroller` (see `utils/README.md`).

## Coding Style & Naming Conventions
- C++17 is required (`CMakeLists.txt` enforces this).
- Indentation is 2 spaces; keep brace style consistent with existing `.cc` files.
- Source files use lower-kebab names like `bus-towards-oval-line.cc` with headers in `include/`.
- No formatter is enforced; keep diffs minimal and follow nearby style.

## Testing Guidelines
- There is no dedicated test suite; validate changes by running `ScreenTest` or a utility with real hardware.
- Use `--led-*` flags (documented in `README.md`) to match panel configuration.

## Commit & Pull Request Guidelines
- Commit messages are short, imperative, and usually without prefixes (e.g., "Fixing conan", "Remove GPIO usage").
- PRs should describe the hardware setup tested, the command used, and include relevant runtime flags.

## Hardware & Configuration Notes
- Most binaries require root to access GPIO; use `sudo` when running on a Pi.
- Use a Pi Zero W Conan host profile (ARMv6 + sysroot/toolchain) when cross-building off-device.
- Dependency inputs are tracked in `conanfile.txt` and `CMakePresets.json` for reproducible builds.

Example `~/.conan2/profiles/pi-zero-w`:
```ini
[settings]
os=Linux
arch=armv6
compiler=gcc
compiler.version=10
compiler.libcxx=libstdc++11
build_type=Release

[conf]
tools.build:sysroot=/opt/rpi/sysroot
tools.build:compiler_executables={"c":"arm-linux-gnueabihf-gcc","cpp":"arm-linux-gnueabihf-g++"}
```
