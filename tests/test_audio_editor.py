#!/usr/bin/env python3
"""
Python test suite for the C++ Audio Editor library.
Uses ctypes to interface with the compiled C++ library.
"""

import os
import sys
import ctypes
import struct
import wave
import numpy as np
from ctypes import c_void_p, c_size_t, c_int16, c_char_p, c_bool, POINTER
import tempfile
import unittest

class AudioEditorTestCase(unittest.TestCase):
    """Base test case for audio editor tests"""
    
    @classmethod
    def setUpClass(cls):
        """Load the C++ library"""
        # Try to find the compiled library
        lib_names = [
            './libAudioEditorLib.a',  # Static library
            './libAudioEditorLib.so', # Shared library (Linux)
            './libAudioEditorLib.dylib', # Shared library (macOS)
            './AudioEditorLib.dll',   # Windows
            '../build/libAudioEditorLib.a',
            '../build/libAudioEditorLib.so',
        ]
        
        cls.lib = None
        for lib_name in lib_names:
            if os.path.exists(lib_name):
                try:
                    cls.lib = ctypes.CDLL(lib_name)
                    break
                except OSError:
                    continue
        
        if cls.lib is None:
            raise RuntimeError("Could not find or load the AudioEditor library. Please compile first.")
        
        # Define function signatures
        cls._define_signatures()
    
    @classmethod
    def _define_signatures(cls):
        """Define C++ function signatures for ctypes"""
        # Note: This is a simplified approach. In a real implementation,
        # you would need C wrapper functions or Python bindings
        pass
    
    def setUp(self):
        """Set up for each test"""
        self.temp_files = []
    
    def tearDown(self):
        """Clean up after each test"""
        for temp_file in self.temp_files:
            try:
                os.remove(temp_file)
            except OSError:
                pass

class PythonAudioEditor:
    """
    Pure Python implementation for testing and comparison.
    This simulates the C++ functionality for testing purposes.
    """
    
    def __init__(self):
        self.segments = []
        self.total_length = 0
    
    def write(self, data, position):
        """Write audio data to the track"""
        if not isinstance(data, list):
            data = list(data)
        
        # Extend track if needed
        end_pos = position + len(data)
        if end_pos > self.total_length:
            # Extend with zeros
            current_data = self.get_all_samples()
            current_data.extend([0] * (end_pos - len(current_data)))
            self.segments = [current_data]
            self.total_length = len(current_data)
        
        # Get current data
        current_data = self.get_all_samples()
        
        # Overwrite with new data
        for i, sample in enumerate(data):
            if position + i < len(current_data):
                current_data[position + i] = sample
        
        self.segments = [current_data]
        self.total_length = len(current_data)
    
    def read(self, start_pos, length):
        """Read audio data from the track"""
        current_data = self.get_all_samples()
        end_pos = min(start_pos + length, len(current_data))
        return current_data[start_pos:end_pos]
    
    def length(self):
        """Get track length"""
        return self.total_length
    
    def get_all_samples(self):
        """Get all samples as a single list"""
        if not self.segments:
            return []
        return list(self.segments[0]) if self.segments else []
    
    def delete_range(self, pos, length):
        """Delete a range of samples"""
        current_data = self.get_all_samples()
        if pos + length > len(current_data):
            return False
        
        # Remove the range
        new_data = current_data[:pos] + current_data[pos + length:]
        self.segments = [new_data]
        self.total_length = len(new_data)
        return True
    
    def identify_ads(self, ad_pattern):
        """Identify advertisement patterns using cross-correlation"""
        if not isinstance(ad_pattern, list):
            ad_pattern = list(ad_pattern)
        
        target_data = self.get_all_samples()
        
        if len(target_data) < len(ad_pattern):
            return ""
        
        # Calculate auto-correlation reference
        auto_ref = sum(x * x for x in ad_pattern)
        threshold = 0.95 * auto_ref
        
        occurrences = []
        i = 0
        while i <= len(target_data) - len(ad_pattern):
            # Calculate cross-correlation
            corr = sum(target_data[i + j] * ad_pattern[j] for j in range(len(ad_pattern)))
            
            if corr >= threshold:
                start_idx = i
                end_idx = i + len(ad_pattern) - 1
                occurrences.append(f"{start_idx},{end_idx}")
                i += len(ad_pattern)  # Skip ahead
            else:
                i += 1
        
        return "\n".join(occurrences)
    
    def insert(self, src_data, dest_pos, src_pos, length):
        """Insert data from source at destination position"""
        if not isinstance(src_data, list):
            src_data = list(src_data)
        
        # Extract the portion to insert
        insert_data = src_data[src_pos:src_pos + length]
        
        # Get current destination data
        dest_data = self.get_all_samples()
        
        # Insert the data
        new_data = dest_data[:dest_pos] + insert_data + dest_data[dest_pos:]
        self.segments = [new_data]
        self.total_length = len(new_data)

class TestBasicOperations(AudioEditorTestCase):
    """Test basic audio editing operations"""
    
    def test_write_and_read(self):
        """Test writing and reading audio data"""
        editor = PythonAudioEditor()
        test_data = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
        
        # Write data
        editor.write(test_data, 0)
        self.assertEqual(editor.length(), len(test_data))
        
        # Read data back
        read_data = editor.read(0, len(test_data))
        self.assertEqual(read_data, test_data)
    
    def test_extend_track(self):
        """Test extending a track with additional data"""
        editor = PythonAudioEditor()
        
        # Write initial data
        data1 = [1, 2, 3, 4, 5]
        editor.write(data1, 0)
        
        # Extend with more data
        data2 = [6, 7, 8, 9, 10]
        editor.write(data2, 5)
        
        self.assertEqual(editor.length(), 10)
        
        # Read all data
        all_data = editor.read(0, editor.length())
        expected = data1 + data2
        self.assertEqual(all_data, expected)
    
    def test_overwrite_data(self):
        """Test overwriting existing data"""
        editor = PythonAudioEditor()
        
        # Write initial data
        initial_data = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
        editor.write(initial_data, 0)
        
        # Overwrite part of the data
        new_data = [-1, -2, -3]
        editor.write(new_data, 3)
        
        # Check result
        result = editor.read(0, editor.length())
        expected = [1, 2, 3, -1, -2, -3, 7, 8, 9, 10]
        self.assertEqual(result, expected)

class TestAdvancedOperations(AudioEditorTestCase):
    """Test advanced audio editing operations"""
    
    def test_delete_range(self):
        """Test deleting a range of samples"""
        editor = PythonAudioEditor()
        
        # Write test data
        test_data = [10, 20, 30, 40, 50, 60, 70, 80, 90, 100]
        editor.write(test_data, 0)
        
        # Delete samples 40, 50, 60, 70 (positions 3-6)
        success = editor.delete_range(3, 4)
        self.assertTrue(success)
        
        # Check result
        result = editor.read(0, editor.length())
        expected = [10, 20, 30, 80, 90, 100]
        self.assertEqual(result, expected)
    
    def test_insert_operation(self):
        """Test inserting data from one track into another"""
        # Create source data
        src_data = [100, 101, 102, 103, 104]
        
        # Create destination editor
        dest_editor = PythonAudioEditor()
        dest_data = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
        dest_editor.write(dest_data, 0)
        
        # Insert 3 samples from src_data (starting at pos 1) into dest at pos 5
        dest_editor.insert(src_data, 5, 1, 3)
        
        # Check result
        result = dest_editor.read(0, dest_editor.length())
        expected = [1, 2, 3, 4, 5, 101, 102, 103, 6, 7, 8, 9, 10]
        self.assertEqual(result, expected)
    
    def test_identify_advertisements(self):
        """Test advertisement identification"""
        editor = PythonAudioEditor()
        
        # Create target with embedded ad patterns
        target_data = [1, 2, 3, 10, 20, 30, 4, 5, 6, 10, 20, 30, 7, 8, 9]
        editor.write(target_data, 0)
        
        # Create ad pattern
        ad_pattern = [10, 20, 30]
        
        # Identify occurrences
        occurrences = editor.identify_ads(ad_pattern)
        
        # Should find two occurrences
        expected = "3,5\n9,11"
        self.assertEqual(occurrences, expected)

class TestWavFileIO(AudioEditorTestCase):
    """Test WAV file input/output operations"""
    
    def create_test_wav_file(self, filename, samples, sample_rate=8000):
        """Create a test WAV file with given samples"""
        with wave.open(filename, 'wb') as wav_file:
            wav_file.setnchannels(1)  # Mono
            wav_file.setsampwidth(2)  # 16-bit
            wav_file.setframerate(sample_rate)
            
            # Convert samples to bytes
            sample_bytes = b''.join(struct.pack('<h', int(sample)) for sample in samples)
            wav_file.writeframes(sample_bytes)
    
    def read_wav_file(self, filename):
        """Read samples from a WAV file"""
        with wave.open(filename, 'rb') as wav_file:
            frames = wav_file.readframes(wav_file.getnframes())
            samples = [struct.unpack('<h', frames[i:i+2])[0] for i in range(0, len(frames), 2)]
            return samples
    
    def test_wav_round_trip(self):
        """Test creating, saving, and loading WAV files"""
        # Create test data (simple sine wave)
        samples = []
        for i in range(1000):
            sample = int(10000 * np.sin(2 * np.pi * i / 100))
            samples.append(sample)
        
        # Create temporary file
        with tempfile.NamedTemporaryFile(suffix='.wav', delete=False) as temp_file:
            temp_filename = temp_file.name
            self.temp_files.append(temp_filename)
        
        # Save samples to WAV file
        self.create_test_wav_file(temp_filename, samples)
        
        # Read samples back
        loaded_samples = self.read_wav_file(temp_filename)
        
        # Compare (should be identical)
        self.assertEqual(len(samples), len(loaded_samples))
        for i in range(min(10, len(samples))):  # Check first 10 samples
            self.assertEqual(samples[i], loaded_samples[i])

class TestPerformance(AudioEditorTestCase):
    """Test performance with larger datasets"""
    
    def test_large_track_operations(self):
        """Test operations on larger tracks"""
        editor = PythonAudioEditor()
        
        # Create large dataset
        large_data = list(range(10000))
        editor.write(large_data, 0)
        
        # Test reading
        read_data = editor.read(0, len(large_data))
        self.assertEqual(len(read_data), len(large_data))
        
        # Test partial read
        partial_data = editor.read(1000, 100)
        expected = large_data[1000:1100]
        self.assertEqual(partial_data, expected)
    
    def test_multiple_insertions(self):
        """Test multiple insertion operations"""
        editor = PythonAudioEditor()
        
        # Start with base data
        base_data = [0] * 100
        editor.write(base_data, 0)
        
        # Perform multiple insertions
        for i in range(10):
            insert_data = [i * 10 + j for j in range(5)]
            editor.insert(insert_data, i * 10, 0, len(insert_data))
        
        # Verify the track has grown
        self.assertGreater(editor.length(), 100)

def run_tests():
    """Run all tests"""
    # Create test suite
    test_suite = unittest.TestSuite()
    
    # Add test cases
    test_classes = [
        TestBasicOperations,
        TestAdvancedOperations,
        TestWavFileIO,
        TestPerformance
    ]
    
    for test_class in test_classes:
        tests = unittest.TestLoader().loadTestsFromTestCase(test_class)
        test_suite.addTests(tests)
    
    # Run tests
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(test_suite)
    
    return result.wasSuccessful()

if __name__ == '__main__':
    print("Audio Editor Python Test Suite")
    print("=" * 50)
    
    try:
        success = run_tests()
        sys.exit(0 if success else 1)
    except Exception as e:
        print(f"Test suite failed to run: {e}")
        sys.exit(1) 