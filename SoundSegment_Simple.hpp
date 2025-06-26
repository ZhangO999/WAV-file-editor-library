#ifndef SOUND_SEGMENT_SIMPLE_HPP
#define SOUND_SEGMENT_SIMPLE_HPP

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

namespace AudioEditor {

constexpr size_t WAV_HEADER_SIZE = 44;
constexpr double CORRELATION_THRESHOLD = 0.95;
constexpr uint32_t SAMPLE_RATE = 8000;
constexpr uint16_t BITS_PER_SAMPLE = 16;
constexpr uint16_t NUM_CHANNELS = 1;
constexpr uint16_t PCM_FORMAT = 1;
constexpr uint16_t PCM_HEADER_SIZE = 16;
constexpr size_t BYTES_PER_SAMPLE = 2;

// Forward declarations
class SegmentNode;
class SoundSegment;

/**
 * A node in the linked list of audio segments.
 */
class SegmentNode {
public:
    int16_t* data;              // Pointer to audio data
    size_t offset;              // Offset into shared data buffer
    size_t length;              // Number of samples in this segment
    size_t global_start;        // Starting global index of this node's samples
    SegmentNode* next;          // Pointer to the next segment
    bool is_buffer_owner;       // True if this node owns the buffer

    SegmentNode();
    SegmentNode(int16_t* data_ptr, size_t offset, size_t len);
    ~SegmentNode();
};

/**
 * Simple vector-like container for int16_t
 */
class SimpleVector {
private:
    int16_t* data_;
    size_t size_;
    size_t capacity_;

public:
    SimpleVector();
    SimpleVector(size_t initial_size);
    SimpleVector(const SimpleVector& other);
    SimpleVector& operator=(const SimpleVector& other);
    ~SimpleVector();

    void push_back(int16_t value);
    void resize(size_t new_size);
    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }
    
    int16_t& operator[](size_t index) { return data_[index]; }
    const int16_t& operator[](size_t index) const { return data_[index]; }
    
    int16_t* data() { return data_; }
    const int16_t* data() const { return data_; }
};

/**
 * Simple string class
 */
class SimpleString {
private:
    char* data_;
    size_t length_;

public:
    SimpleString();
    SimpleString(const char* str);
    SimpleString(const SimpleString& other);
    SimpleString& operator=(const SimpleString& other);
    ~SimpleString();

    const char* c_str() const { return data_ ? data_ : ""; }
    size_t length() const { return length_; }
    bool empty() const { return length_ == 0; }
    
    SimpleString& operator+=(const SimpleString& other);
    bool operator==(const SimpleString& other) const;
};

/**
 * WAV file I/O utility class
 */
class WavIO {
public:
    static SimpleVector load(const char* filename);
    static void save(const char* filename, const SimpleVector& samples);
};

/**
 * The main SoundSegment class representing a sequence of audio segments.
 */
class SoundSegment {
private:
    SegmentNode* head;          // Pointer to the first segment
    size_t total_length;        // Total number of samples in the track

    // Helper methods
    void updateGlobalIndices();
    SegmentNode* findSegmentAt(size_t pos, size_t& local_offset) const;
    SegmentNode* createSegment(int16_t* data, size_t offset, size_t len);

public:
    // Constructors and destructor
    SoundSegment();
    ~SoundSegment();
    
    // Disable copy constructor and assignment operator for simplicity
    SoundSegment(const SoundSegment& other) = delete;
    SoundSegment& operator=(const SoundSegment& other) = delete;

    // Basic track operations
    size_t length() const;
    void read(SimpleVector& dest, size_t start_pos, size_t len) const;
    void read(int16_t* dest, size_t start_pos, size_t len) const;
    void write(const SimpleVector& src, size_t pos);
    void write(const int16_t* src, size_t pos, size_t len);
    
    // Advanced operations
    bool deleteRange(size_t pos, size_t len);
    SimpleString identify(const SoundSegment& ad) const;
    void insert(const SoundSegment& src_track, size_t dest_pos, size_t src_pos, size_t len);
    
    // WAV file operations
    void loadFromWav(const char* filename);
    void saveToWav(const char* filename) const;
    
    // Utility methods for testing and debugging
    void printTrack() const;
    SimpleVector getAllSamples() const;
    
    // Static factory method
    static SoundSegment* create();
};

}  // namespace AudioEditor

#endif  // SOUND_SEGMENT_SIMPLE_HPP 