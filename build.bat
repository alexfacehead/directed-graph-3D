@echo off
echo Building directed-graph-3D...
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
echo.
echo Done! Run with: build\Release\directed_graph.exe
