#!/usr/bin/env bash
set -e
echo "Building directed-graph-3D..."
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
echo ""
echo "Done! Run with: ./build/directed_graph"
