#include "SoundSegment.hpp"
#include <iostream>
#include <vector>
#include <cassert>
#include <cmath>

using namespace AudioEditor;

void test_basic_operations() {
    std::cout << "=== Test 1: Basic Write and Read Operations ===\n";
    
    auto track = SoundSegment::create();
    std::vector<int16_t> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    // Write data to track
    track->write(data, 0);
    std::cout << "Track length after write: " << track->length() << "\n";
    
    // Read data back
    std::vector<int16_t> read_data;
    track->read(read_data, 0, track->length());
    
    std::cout << "Read back data: ";
    for (const auto& sample : read_data) {
        std::cout << sample << " ";
    }
    std::cout << "\n";
    
    track->printTrack();
}

void test_extend_track() {
    std::cout << "\n=== Test 2: Extend Track ===\n";
    
    auto track = SoundSegment::create();
    std::vector<int16_t> data1 = {1, 2, 3, 4, 5};
    std::vector<int16_t> data2 = {6, 7, 8, 9, 10};
    
    track->write(data1, 0);
    track->write(data2, 5);  // Extend at the end
    
    std::cout << "Track length after extension: " << track->length() << "\n";
    track->printTrack();
}

void test_delete_range() {
    std::cout << "\n=== Test 3: Delete Range ===\n";
    
    auto track = SoundSegment::create();
    std::vector<int16_t> data = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    
    track->write(data, 0);
    std::cout << "Before deletion:\n";
    track->printTrack();
    
    // Delete samples 40, 50, 60, 70 (positions 3-6)
    bool success = track->deleteRange(3, 4);
    std::cout << "Deletion success: " << (success ? "true" : "false") << "\n";
    std::cout << "After deletion:\n";
    track->printTrack();
}

void test_insert() {
    std::cout << "\n=== Test 4: Insert Operation ===\n";
    
    // Create source track
    auto src_track = SoundSegment::create();
    std::vector<int16_t> src_data = {100, 101, 102, 103, 104};
    src_track->write(src_data, 0);
    
    // Create destination track
    auto dest_track = SoundSegment::create();
    std::vector<int16_t> dest_data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    dest_track->write(dest_data, 0);
    
    std::cout << "Before insertion:\n";
    std::cout << "Source track:\n";
    src_track->printTrack();
    std::cout << "Destination track:\n";
    dest_track->printTrack();
    
    // Insert 3 samples from src_track (starting at pos 1) into dest_track at pos 5
    dest_track->insert(*src_track, 5, 1, 3);
    
    std::cout << "After insertion:\n";
    std::cout << "Destination track:\n";
    dest_track->printTrack();
}

void test_identify_ads() {
    std::cout << "\n=== Test 5: Advertisement Identification ===\n";
    
    // Create target track with some data
    auto target = SoundSegment::create();
    std::vector<int16_t> target_data = {1, 2, 3, 10, 20, 30, 4, 5, 6, 10, 20, 30, 7, 8, 9};
    target->write(target_data, 0);
    
    // Create ad pattern
    auto ad = SoundSegment::create();
    std::vector<int16_t> ad_data = {10, 20, 30};
    ad->write(ad_data, 0);
    
    std::cout << "Target track:\n";
    target->printTrack();
    std::cout << "Ad pattern:\n";
    ad->printTrack();
    
    // Identify ad occurrences
    std::string occurrences = target->identify(*ad);
    std::cout << "Ad occurrences: " << occurrences << "\n";
}

void test_wav_io() {
    std::cout << "\n=== Test 6: WAV File I/O ===\n";
    
    try {
        // Create a simple sine wave pattern
        auto track = SoundSegment::create();
        std::vector<int16_t> sine_wave;
        
        for (int i = 0; i < 1000; ++i) {
            // Simple sine wave approximation
            double sample = 10000.0 * sin(2.0 * 3.14159 * i / 100.0);
            sine_wave.push_back(static_cast<int16_t>(sample));
        }
        
        track->write(sine_wave, 0);
        
        // Save to WAV file
        track->saveToWav("test_output.wav");
        std::cout << "Saved track to test_output.wav\n";
        
        // Create new track and load from WAV file
        auto loaded_track = SoundSegment::create();
        loaded_track->loadFromWav("test_output.wav");
        
        std::cout << "Loaded track length: " << loaded_track->length() << "\n";
        std::cout << "Original track length: " << track->length() << "\n";
        
        // Compare first few samples
        auto original_samples = track->getAllSamples();
        auto loaded_samples = loaded_track->getAllSamples();
        
        bool match = (original_samples.size() == loaded_samples.size());
        if (match) {
            for (size_t i = 0; i < std::min(size_t(10), original_samples.size()); ++i) {
                if (original_samples[i] != loaded_samples[i]) {
                    match = false;
                    break;
                }
            }
        }
        
        std::cout << "First 10 samples match: " << (match ? "true" : "false") << "\n";
        
    } catch (const std::exception& e) {
        std::cout << "WAV I/O test failed: " << e.what() << "\n";
    }
}

int main() {
    std::cout << "C++ Audio Editor Demo\n";
    std::cout << "=====================\n";
    
    test_basic_operations();
    test_extend_track();
    test_delete_range();
    test_insert();
    test_identify_ads();
    test_wav_io();
    
    std::cout << "\nAll tests completed!\n";
    return 0;
} 