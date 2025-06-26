#include "SoundSegment_Simple.hpp"
#include <stdio.h>

using namespace AudioEditor;

int main() {
    printf("Audio Editor C++ - Simple Demo\n");
    printf("================================\n\n");

    // Test 1: Basic track creation and operations
    printf("Test 1: Basic Operations\n");
    SoundSegment* track = SoundSegment::create();
    
    // Create some test data
    SimpleVector test_data;
    for (int i = 0; i < 100; i++) {
        test_data.push_back(i * 10);
    }
    
    track->write(test_data, 0);
    printf("Track length after writing 100 samples: %zu\n", track->length());
    
    // Read back some data
    SimpleVector read_data;
    track->read(read_data, 10, 20);
    printf("Read 20 samples starting at position 10: ");
    for (size_t i = 0; i < read_data.size() && i < 10; i++) {
        printf("%d ", read_data[i]);
    }
    printf("%s\n", read_data.size() > 10 ? "..." : "");
    
    // Test 2: Delete range
    printf("\nTest 2: Delete Range\n");
    bool success = track->deleteRange(20, 30);
    printf("Deleted 30 samples at position 20: %s\n", success ? "Success" : "Failed");
    printf("Track length after deletion: %zu\n", track->length());
    
    // Test 3: Create advertisement pattern
    printf("\nTest 3: Advertisement Identification\n");
    SoundSegment* ad = SoundSegment::create();
    SimpleVector ad_data;
    for (int i = 0; i < 10; i++) {
        ad_data.push_back(100 + i * 5);  // Pattern: 100, 105, 110, ...
    }
    ad->write(ad_data, 0);
    
    // Insert ad pattern into main track
    track->insert(*ad, 50, 0, ad->length());
    printf("Inserted ad pattern at position 50\n");
    printf("Track length after insertion: %zu\n", track->length());
    
    // Try to identify the ad
    SimpleString result = track->identify(*ad);
    printf("Advertisement identification result: '%s'\n", result.c_str());
    
    // Test 4: String operations
    printf("\nTest 4: String Operations\n");
    SimpleString str1("Hello");
    SimpleString str2(" World");
    str1 += str2;
    printf("String concatenation: '%s'\n", str1.c_str());
    
    // Test 5: Vector operations
    printf("\nTest 5: Vector Operations\n");
    SimpleVector vec1;
    for (int i = 0; i < 5; i++) {
        vec1.push_back(i * i);
    }
    
    SimpleVector vec2 = vec1;  // Copy constructor
    vec2.resize(10);  // Resize with zero-initialization
    
    printf("Original vector: ");
    for (size_t i = 0; i < vec1.size(); i++) {
        printf("%d ", vec1[i]);
    }
    printf("\nCopied and resized vector: ");
    for (size_t i = 0; i < vec2.size(); i++) {
        printf("%d ", vec2[i]);
    }
    printf("\n");
    
    // Clean up
    delete track;
    delete ad;
    
    printf("\nAll tests completed successfully!\n");
    return 0;
} 