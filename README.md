# C++ Audio Editor

A modern C++ implementation of an audio editing backend with support for WAV file I/O, segment-based audio manipulation, and advanced operations like advertisement detection and shared backing store.

## Overview

This project is a C++ conversion of a C audio editing assignment. It provides a comprehensive audio editing library with the following features:

- **WAV File I/O**: Read and write standard 16-bit mono WAV files
- **Segment-based Architecture**: Efficient audio manipulation using linked segments
- **Shared Backing Store**: Memory-efficient operations with shared data references
- **Advertisement Detection**: Cross-correlation based pattern matching
- **Advanced Operations**: Insert, delete, read, write with complex data sharing

## Architecture

### Core Classes

- **`SoundSegment`**: Main audio track class representing a sequence of audio segments
- **`SegmentNode`**: Individual audio segment with metadata and parent-child relationships
- **`WavIO`**: Utility class for WAV file operations

### Key Features

1. **Memory Efficiency**: Uses shared pointers and smart memory management
2. **Flexible Operations**: Support for insertion, deletion, and complex manipulations
3. **Cross-correlation**: Built-in advertisement detection using signal processing
4. **Object-oriented Design**: Clean C++ implementation with RAII principles

## Building the Project

### Prerequisites

- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.12 or higher
- Optional: Python 3.6+ for testing (with numpy, matplotlib, psutil)

### Build Instructions

```bash
# Clone or download the project
cd audio-editor-cpp

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build the project
make

# Or use CMake to build
cmake --build .
```

### Build Options

```bash
# Build with tests (default: ON)
cmake -DBUILD_TESTS=ON ..

# Build with Python bindings (requires pybind11)
cmake -DBUILD_PYTHON_BINDINGS=ON ..

# Debug build
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Release build (default)
cmake -DCMAKE_BUILD_TYPE=Release ..
```

## Usage

### Basic C++ Usage

```cpp
#include "SoundSegment.hpp"
using namespace AudioEditor;

// Create a new audio track
auto track = SoundSegment::create();

// Write audio data
std::vector<int16_t> samples = {1, 2, 3, 4, 5};
track->write(samples, 0);

// Read audio data
std::vector<int16_t> read_data;
track->read(read_data, 0, track->length());

// Load from WAV file
track->loadFromWav("input.wav");

// Save to WAV file
track->saveToWav("output.wav");

// Advanced operations
auto other_track = SoundSegment::create();
// Insert portion of other_track into track
track->insert(*other_track, dest_pos, src_pos, length);

// Delete range
track->deleteRange(start_pos, length);

// Identify advertisements
std::string occurrences = track->identify(*ad_pattern);
```

### Running the Demo

```bash
# After building
./demo
```

This will run a comprehensive demonstration of all audio editing features.

### Running Tests

#### C++ Tests

```bash
# Run C++ tests
./test_basic

# Or use CMake test runner
ctest
```

#### Python Tests

```bash
# Navigate to tests directory
cd tests

# Run basic tests
python test_audio_editor.py

# Run performance benchmarks
python benchmark.py
```

## API Reference

### SoundSegment Class

#### Constructor and Factory
```cpp
static std::unique_ptr<SoundSegment> create();
```

#### Basic Operations
```cpp
size_t length() const;
void read(std::vector<int16_t>& dest, size_t start_pos, size_t len) const;
void write(const std::vector<int16_t>& src, size_t pos);
```

#### Advanced Operations
```cpp
bool deleteRange(size_t pos, size_t len);
std::string identify(const SoundSegment& ad) const;
void insert(const SoundSegment& src_track, size_t dest_pos, size_t src_pos, size_t len);
```

#### File I/O
```cpp
void loadFromWav(const std::string& filename);
void saveToWav(const std::string& filename) const;
```

### WAV File Format Support

- **Format**: PCM
- **Sample Rate**: 8000 Hz (configurable)
- **Bit Depth**: 16-bit
- **Channels**: Mono (1 channel)

## Testing

The project includes comprehensive test suites:

### C++ Tests (`tests/test_basic.cpp`)
- Basic read/write operations
- Track extension and manipulation
- Delete range operations
- Insert operations
- Advertisement identification
- WAV file I/O
- Edge cases and error handling

### Python Tests (`tests/test_audio_editor.py`)
- Unit tests using Python's unittest framework
- Comparison with pure Python implementation
- WAV file format testing
- Performance testing
- Memory usage validation

### Benchmark Suite (`tests/benchmark.py`)
- Performance benchmarking for all operations
- Memory usage profiling
- Stress testing with large datasets
- Visual performance charts (requires matplotlib)

## Performance Characteristics

### Time Complexity
- **Read**: O(n) where n is the number of segments to traverse
- **Write**: O(n) for extension, O(1) for overwrite
- **Insert**: O(m) where m is the number of segments to copy
- **Delete**: O(n) for validation + O(m) for data movement
- **Identify (Cross-correlation)**: O(n*m) where n is target length, m is ad length

### Memory Usage
- Shared backing store minimizes memory duplication
- Smart pointers ensure automatic memory management
- Weak pointers prevent circular references in parent-child relationships

## Comparison with Original C Implementation

### Improvements
- **Memory Safety**: Smart pointers eliminate manual memory management
- **Type Safety**: Strong typing and const-correctness
- **Exception Safety**: RAII and exception handling
- **Code Organization**: Proper encapsulation and modularity
- **Testing**: Comprehensive test suites with multiple approaches

### Maintained Features
- All original functionality preserved
- Same algorithmic complexity
- Compatible WAV file format
- Equivalent performance characteristics

## Known Limitations

1. **Shared Insertion**: Current implementation creates copies instead of true shared backing for insert operations (safer but less memory efficient)
2. **WAV Format**: Limited to 16-bit mono PCM format
3. **Platform**: Developed and tested primarily on Unix-like systems

## Future Enhancements

- [ ] True shared backing store for insert operations
- [ ] Support for stereo and other WAV formats
- [ ] Python bindings using pybind11
- [ ] Multi-threading support for large operations
- [ ] Additional audio effects and filters
- [ ] Real-time audio processing capabilities

## Contributing

1. Fork the repository
2. Create a feature branch
3. Add tests for new functionality
4. Ensure all tests pass
5. Submit a pull request

## License

This project is provided as-is for educational purposes. Please respect any original assignment guidelines and academic integrity policies.

## Acknowledgments

- Original C implementation as the foundation
- COMP2017/9017 assignment specification for requirements
- Modern C++ best practices and design patterns 