#include "../SoundSegment.hpp"
#include <iostream>
#include <vector>
#include <cassert>
#include <string>
#include <cmath>

using namespace AudioEditor;

// Simple assertion macro for testing
#define ASSERT(condition, message) \
    if (!(condition)) { \
        std::cerr << "ASSERTION FAILED: " << message << " at line " << __LINE__ << std::endl; \
        return false; \
    }

bool test_basic_write_read() {
    std::cout << "Testing basic write/read operations..." << std::endl;
    
    auto track = SoundSegment::create();
    std::vector<int16_t> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    // Write data
    track->write(data, 0);
    ASSERT(track->length() == data.size(), "Track length should match written data size");
    
    // Read data back
    std::vector<int16_t> read_data;
    track->read(read_data, 0, track->length());
    
    ASSERT(read_data.size() == data.size(), "Read data size should match original");
    for (size_t i = 0; i < data.size(); ++i) {
        ASSERT(read_data[i] == data[i], "Read data should match written data");
    }
    
    std::cout << "✓ Basic write/read test passed" << std::endl;
    return true;
}

bool test_extend_track() {
    std::cout << "Testing track extension..." << std::endl;
    
    auto track = SoundSegment::create();
    std::vector<int16_t> data1 = {1, 2, 3, 4, 5};
    std::vector<int16_t> data2 = {6, 7, 8, 9, 10};
    
    track->write(data1, 0);
    track->write(data2, 5);  // Extend at the end
    
    ASSERT(track->length() == 10, "Track should be extended to 10 samples");
    
    auto all_samples = track->getAllSamples();
    std::vector<int16_t> expected = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    ASSERT(all_samples.size() == expected.size(), "Extended track size should be correct");
    for (size_t i = 0; i < expected.size(); ++i) {
        ASSERT(all_samples[i] == expected[i], "Extended track data should be correct");
    }
    
    std::cout << "✓ Track extension test passed" << std::endl;
    return true;
}

bool test_delete_range() {
    std::cout << "Testing delete range operation..." << std::endl;
    
    auto track = SoundSegment::create();
    std::vector<int16_t> data = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    
    track->write(data, 0);
    
    // Delete samples 40, 50, 60, 70 (positions 3-6)
    bool success = track->deleteRange(3, 4);
    ASSERT(success, "Delete range should succeed");
    
    auto result = track->getAllSamples();
    std::vector<int16_t> expected = {10, 20, 30, 80, 90, 100};
    
    ASSERT(result.size() == expected.size(), "Delete range should reduce track size");
    for (size_t i = 0; i < expected.size(); ++i) {
        ASSERT(result[i] == expected[i], "Delete range should preserve correct samples");
    }
    
    std::cout << "✓ Delete range test passed" << std::endl;
    return true;
}

bool test_insert_operation() {
    std::cout << "Testing insert operation..." << std::endl;
    
    // Create source track
    auto src_track = SoundSegment::create();
    std::vector<int16_t> src_data = {100, 101, 102, 103, 104};
    src_track->write(src_data, 0);
    
    // Create destination track
    auto dest_track = SoundSegment::create();
    std::vector<int16_t> dest_data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    dest_track->write(dest_data, 0);
    
    // Insert 3 samples from src_track (starting at pos 1) into dest_track at pos 5
    dest_track->insert(*src_track, 5, 1, 3);
    
    auto result = dest_track->getAllSamples();
    // Expected: [1, 2, 3, 4, 5, 101, 102, 103, 6, 7, 8, 9, 10]
    std::vector<int16_t> expected = {1, 2, 3, 4, 5, 101, 102, 103, 6, 7, 8, 9, 10};
    
    ASSERT(result.size() == expected.size(), "Insert should increase track size correctly");
    for (size_t i = 0; i < expected.size(); ++i) {
        ASSERT(result[i] == expected[i], "Insert should place data correctly");
    }
    
    std::cout << "✓ Insert operation test passed" << std::endl;
    return true;
}

bool test_identify_ads() {
    std::cout << "Testing advertisement identification..." << std::endl;
    
    // Create target track with embedded ad patterns
    auto target = SoundSegment::create();
    std::vector<int16_t> target_data = {1, 2, 3, 10, 20, 30, 4, 5, 6, 10, 20, 30, 7, 8, 9};
    target->write(target_data, 0);
    
    // Create ad pattern
    auto ad = SoundSegment::create();
    std::vector<int16_t> ad_data = {10, 20, 30};
    ad->write(ad_data, 0);
    
    // Identify occurrences
    std::string occurrences = target->identify(*ad);
    
    // Should find two occurrences: positions 3-5 and 9-11
    std::string expected = "3,5\n9,11";
    ASSERT(occurrences == expected, "Should identify correct ad occurrences");
    
    std::cout << "✓ Advertisement identification test passed" << std::endl;
    return true;
}

bool test_wav_io() {
    std::cout << "Testing WAV file I/O..." << std::endl;
    
    try {
        // Create a simple pattern
        auto track = SoundSegment::create();
        std::vector<int16_t> test_data;
        
        for (int i = 0; i < 100; ++i) {
            // Simple pattern
            test_data.push_back(static_cast<int16_t>(i * 100));
        }
        
        track->write(test_data, 0);
        
        // Save to WAV file
        track->saveToWav("test_output.wav");
        
        // Create new track and load from WAV file
        auto loaded_track = SoundSegment::create();
        loaded_track->loadFromWav("test_output.wav");
        
        ASSERT(loaded_track->length() == track->length(), "Loaded track should have same length");
        
        // Compare samples
        auto original_samples = track->getAllSamples();
        auto loaded_samples = loaded_track->getAllSamples();
        
        for (size_t i = 0; i < std::min(size_t(10), original_samples.size()); ++i) {
            ASSERT(original_samples[i] == loaded_samples[i], "WAV I/O should preserve sample data");
        }
        
        std::cout << "✓ WAV I/O test passed" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "WAV I/O test failed with exception: " << e.what() << std::endl;
        return false;
    }
}

bool test_edge_cases() {
    std::cout << "Testing edge cases..." << std::endl;
    
    // Test empty track
    auto empty_track = SoundSegment::create();
    ASSERT(empty_track->length() == 0, "Empty track should have zero length");
    
    // Test reading from empty track
    std::vector<int16_t> empty_read;
    empty_track->read(empty_read, 0, 10);
    ASSERT(empty_read.empty(), "Reading from empty track should return empty data");
    
    // Test writing to position beyond current length
    empty_track->write(std::vector<int16_t>{1, 2, 3}, 5);
    ASSERT(empty_track->length() == 8, "Writing beyond length should extend track");
    
    auto samples = empty_track->getAllSamples();
    ASSERT(samples[5] == 1 && samples[6] == 2 && samples[7] == 3, 
           "Data should be written at correct position");
    
    std::cout << "✓ Edge cases test passed" << std::endl;
    return true;
}

int main() {
    std::cout << "C++ Audio Editor Test Suite" << std::endl;
    std::cout << "===========================" << std::endl;
    
    bool all_passed = true;
    
    // Run all tests
    all_passed &= test_basic_write_read();
    all_passed &= test_extend_track();
    all_passed &= test_delete_range();
    all_passed &= test_insert_operation();
    all_passed &= test_identify_ads();
    all_passed &= test_wav_io();
    all_passed &= test_edge_cases();
    
    std::cout << std::endl;
    if (all_passed) {
        std::cout << "✓ All tests passed!" << std::endl;
        return 0;
    } else {
        std::cout << "✗ Some tests failed!" << std::endl;
        return 1;
    }
} 