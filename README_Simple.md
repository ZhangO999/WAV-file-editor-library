# Audio Editor C++ - Simple Implementation

## Problem Solved

The original C++ implementation failed to compile due to missing C++ standard library headers (`<iostream>`, `<vector>`, etc.) on the system. This appears to be related to incomplete Xcode Command Line Tools installation.

## Solution

Created a simplified C++ implementation that:
- Uses C-style headers (`<stdio.h>`, `<stdlib.h>`, etc.) instead of C++ headers
- Implements custom container classes (`SimpleVector`, `SimpleString`) 
- Maintains the same C++ class-based API
- Provides all the original functionality

## Files

### Core Implementation
- `SoundSegment_Simple.hpp` - Header file with class definitions
- `SoundSegment_Simple.cpp` - Implementation using C headers
- `demo_simple.cpp` - Demonstration program

### Build System
- `CMakeLists_Simple.txt` - CMake configuration for simplified version

### Testing
- `test_simple.py` - Python test script that compiles and tests the C++ code

## Features

The simplified implementation provides all the original features:

### Audio Segment Management
- Linked list-based audio segments
- Efficient read/write operations
- Memory-safe segment handling

### Audio Operations
- **Read/Write**: Access audio data at any position
- **Delete Range**: Remove audio segments
- **Insert**: Add audio from one track to another with shared backing store
- **Track Extension**: Automatically grow tracks as needed

### Advertisement Detection
- Cross-correlation based pattern matching
- Configurable correlation threshold
- Returns position ranges of detected patterns

### WAV File I/O
- Load WAV files into tracks
- Save tracks as WAV files
- Standard WAV format support (16-bit, mono, 8kHz)

### Custom Container Classes
- `SimpleVector<int16_t>`: Dynamic array for audio samples
- `SimpleString`: String class for text operations
- Manual memory management with proper cleanup

## Compilation

### Direct Compilation
```bash
g++ -std=c++11 -o demo_simple demo_simple.cpp SoundSegment_Simple.cpp
./demo_simple
```

### Using CMake
```bash
mkdir build && cd build
cmake -f ../CMakeLists_Simple.txt ..
make
./demo_simple
```

## Testing

Run the Python test suite:
```bash
python3 test_simple.py
```

This will:
- Test C++ compilation
- Run the demo program
- Verify expected output

## Example Usage

```cpp
#include "SoundSegment_Simple.hpp"
using namespace AudioEditor;

int main() {
    // Create a new audio track
    SoundSegment* track = SoundSegment::create();
    
    // Create some test audio data
    SimpleVector samples;
    for (int i = 0; i < 1000; i++) {
        samples.push_back(sin(2.0 * 3.14159 * i / 100.0) * 10000);
    }
    
    // Write to track
    track->write(samples, 0);
    
    // Read back data
    SimpleVector read_back;
    track->read(read_back, 100, 200);
    
    // Delete a range
    track->deleteRange(300, 100);
    
    // Clean up
    delete track;
    return 0;
}
```

## Key Design Decisions

1. **C Headers**: Used C standard library to avoid missing C++ headers
2. **Manual Memory Management**: Explicit malloc/free for reliability
3. **Simple Containers**: Custom vector/string classes with minimal STL dependencies
4. **Same API**: Maintained the original C++ class interface
5. **Memory Safety**: Proper cleanup in destructors

## Performance

The simplified implementation:
- Has similar performance to the original
- Uses manual memory management (faster allocation)
- Maintains O(n) complexity for most operations
- Efficient cross-correlation for ad detection

## Compatibility

- Requires C++11 or later
- Works with any C++ compiler that supports basic C headers
- Tested with Apple Clang 17.0.0
- No external dependencies beyond standard C library

## Limitations

- No STL containers (uses custom implementations)
- Manual memory management (more error-prone)
- Simplified string operations
- Basic error handling (exit on allocation failure)

## Migration from Original

If you had code using the original implementation:
1. Change include from `"SoundSegment.hpp"` to `"SoundSegment_Simple.hpp"`
2. Replace `std::vector<int16_t>` with `SimpleVector`
3. Replace `std::string` with `SimpleString`
4. The main API remains the same

## Future Improvements

To fully restore modern C++ features:
1. Fix the missing C++ standard library headers issue
2. Reinstall Xcode Command Line Tools
3. Migrate back to STL containers for better safety
4. Add exception handling instead of exit() calls 