#!/bin/bash

# Audio Editor C++ - Build and Test Script
set -e

echo "Audio Editor C++ - Build and Test Script"
echo "========================================"

# Check if CMake is available
if ! command -v cmake &> /dev/null; then
    echo "Error: CMake is required but not installed."
    exit 1
fi

# Check for C++ compiler
if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
    echo "Error: No C++ compiler found (g++ or clang++ required)."
    exit 1
fi

# Clean previous build
echo "Cleaning previous build..."
rm -rf build

# Create build directory
echo "Creating build directory..."
mkdir -p build
cd build

# Configure with CMake
echo "Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON

# Build the project
echo "Building the project..."
make -j$(nproc 2>/dev/null || echo 4)

# Run C++ tests
echo ""
echo "Running C++ tests..."
echo "==================="
if [ -f "./test_basic" ]; then
    ./test_basic
else
    echo "Warning: C++ test executable not found"
fi

# Run the demo
echo ""
echo "Running demo..."
echo "==============="
if [ -f "./demo" ]; then
    ./demo
else
    echo "Warning: Demo executable not found"
fi

# Go back to root directory for Python tests
cd ..

# Check if Python is available
if command -v python3 &> /dev/null; then
    echo ""
    echo "Running Python tests..."
    echo "======================"
    
    # Check if numpy is available
    if python3 -c "import numpy" 2>/dev/null; then
        cd tests
        python3 test_audio_editor.py
        
        # Run benchmarks if optional packages are available
        echo ""
        echo "Running benchmarks..."
        echo "===================="
        if python3 -c "import matplotlib, psutil" 2>/dev/null; then
            python3 benchmark.py
        else
            echo "Skipping benchmarks (matplotlib or psutil not available)"
        fi
        cd ..
    else
        echo "Skipping Python tests (numpy not available)"
    fi
else
    echo "Python3 not found, skipping Python tests"
fi

echo ""
echo "Build and test completed successfully!"
echo ""
echo "Files created:"
echo "- build/demo          - C++ demo executable"
echo "- build/test_basic    - C++ test executable"
echo "- build/libAudioEditorLib.a - Static library"
echo ""
echo "To run individual components:"
echo "- ./build/demo"
echo "- ./build/test_basic"
echo "- cd tests && python3 test_audio_editor.py"
echo "- cd tests && python3 benchmark.py" 