#ifndef SOUND_SEGMENT_HPP
#define SOUND_SEGMENT_HPP

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace AudioEditor {

constexpr size_t WAV_HEADER_SIZE = 44;
constexpr size_t INITIAL_ARRAY_CAPACITY = 2;
constexpr double CORRELATION_THRESHOLD = 0.95;
constexpr uint32_t SAMPLE_RATE = 8000;
constexpr uint16_t BITS_PER_SAMPLE = 16;
constexpr uint16_t NUM_CHANNELS = 1;
constexpr uint16_t PCM_FORMAT = 1;
constexpr uint16_t PCM_HEADER_SIZE = 16;
constexpr size_t BYTES_PER_SAMPLE = 2;
constexpr size_t INITIAL_OFFSET = 0;
constexpr size_t MAX_OCCURRENCE_STRING_LENGTH = 32;

// Forward declarations
class SegmentNode;
class SoundSegment;

/**
 * A node in the linked list of audio segments.
 * Each node represents a contiguous block of audio samples with metadata.
 */
class SegmentNode {
public:
    std::shared_ptr<std::vector<int16_t>> data;  // Shared pointer to audio data
    size_t offset;                               // Offset into shared data buffer
    size_t length;                               // Number of samples in this segment
    size_t global_start;                         // Starting global index of this node's samples
    std::shared_ptr<SegmentNode> next;           // Pointer to the next segment

    std::vector<std::weak_ptr<SegmentNode>> parents;   // Parent nodes
    std::vector<std::weak_ptr<SegmentNode>> children;  // Child nodes
    bool is_buffer_owner;                               // True if this node owns the buffer

    SegmentNode();
    SegmentNode(std::shared_ptr<std::vector<int16_t>> data_ptr, size_t offset, size_t len);
    ~SegmentNode() = default;

    // Add parent-child relationships
    void addChild(std::shared_ptr<SegmentNode> child);
    void addParent(std::shared_ptr<SegmentNode> parent);
    
    // Check if node has active children (for deletion checks)
    bool hasActiveChildren() const;
};

/**
 * WAV file I/O utility class
 */
class WavIO {
public:
    static std::vector<int16_t> load(const std::string& filename);
    static void save(const std::string& filename, const std::vector<int16_t>& samples);
};

/**
 * The main SoundSegment class representing a sequence of audio segments.
 * Replaces the C struct sound_seg with an object-oriented design.
 */
class SoundSegment {
private:
    std::shared_ptr<SegmentNode> head;  // Pointer to the first segment
    size_t total_length;                // Total number of samples in the track

    // Helper methods
    void updateGlobalIndices();
    std::shared_ptr<SegmentNode> findSegmentAt(size_t pos, size_t& local_offset) const;
    std::shared_ptr<SegmentNode> splitNode(std::shared_ptr<SegmentNode> node, size_t local_offset);
    std::shared_ptr<SegmentNode> createSegment(std::shared_ptr<std::vector<int16_t>> data, 
                                               size_t offset, size_t len);

public:
    // Constructors and destructor
    SoundSegment();
    ~SoundSegment() = default;
    
    // Copy constructor and assignment operator
    SoundSegment(const SoundSegment& other) = delete;
    SoundSegment& operator=(const SoundSegment& other) = delete;
    
    // Move constructor and assignment operator
    SoundSegment(SoundSegment&& other) noexcept;
    SoundSegment& operator=(SoundSegment&& other) noexcept;

    // Basic track operations
    size_t length() const;
    void read(std::vector<int16_t>& dest, size_t start_pos, size_t len) const;
    void read(int16_t* dest, size_t start_pos, size_t len) const;
    void write(const std::vector<int16_t>& src, size_t pos);
    void write(const int16_t* src, size_t pos, size_t len);
    
    // Advanced operations
    bool deleteRange(size_t pos, size_t len);
    std::string identify(const SoundSegment& ad) const;
    void insert(const SoundSegment& src_track, size_t dest_pos, size_t src_pos, size_t len);
    
    // WAV file operations
    void loadFromWav(const std::string& filename);
    void saveToWav(const std::string& filename) const;
    
    // Utility methods for testing and debugging
    void printTrack() const;
    std::vector<int16_t> getAllSamples() const;
    
    // Static factory method
    static std::unique_ptr<SoundSegment> create();
};

}  // namespace AudioEditor

#endif  // SOUND_SEGMENT_HPP 