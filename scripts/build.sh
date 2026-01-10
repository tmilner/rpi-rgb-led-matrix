#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'USAGE'
Usage: scripts/build.sh [--on-device]

Builds the project using Conan + CMake.

Options:
  --on-device   Stop the systemd service, git pull, build, then restart and
                monitor service status for 30 seconds.

Environment:
  SERVICE_NAME  Optional systemd unit name (default: led-matrix.service).
  BUILD_TYPE    Optional CMake build type (default: Release).
USAGE
}

on_device=false
for arg in "$@"; do
  case "$arg" in
    --on-device)
      on_device=true
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "Unknown argument: $arg" >&2
      usage
      exit 1
      ;;
  esac
done

build_type="${BUILD_TYPE:-Release}"
service_name="${SERVICE_NAME:-led-matrix.service}"

if $on_device; then
  echo "Stopping service: ${service_name}"
  sudo systemctl stop "${service_name}"

  echo "Updating repo (git pull --ff-only)"
  git pull --ff-only
fi

mkdir -p build

if command -v conan >/dev/null 2>&1; then
  conan install . -of build -s build_type="${build_type}" --build=missing
else
  echo "Conan not found in PATH. Please install conan first." >&2
  exit 1
fi

cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake \
  -DCMAKE_BUILD_TYPE="${build_type}"

cmake --build build

if $on_device; then
  echo "Starting service: ${service_name}"
  sudo systemctl start "${service_name}"

  echo "Monitoring ${service_name} for 30 seconds..."
  for i in $(seq 1 30); do
    if systemctl is-active --quiet "${service_name}"; then
      echo "[$i/30] active"
    else
      echo "[$i/30] not active"
      sudo systemctl --no-pager status "${service_name}" || true
      exit 1
    fi
    sleep 1
  done
fi
