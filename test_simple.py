#!/usr/bin/env python3
"""
Python test script 
"""

import subprocess
import os
import sys

def run_command(cmd):
    """Run a shell command and return the result."""
    try:
        result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
        return result.returncode, result.stdout, result.stderr
    except Exception as e:
        return -1, "", str(e)

def test_compilation():
    """Test that the C++ code compiles successfully."""
    print("Testing C++ compilation...")
    
    returncode, stdout, stderr = run_command(
        "g++ -std=c++11 -o test_compile demo_simple.cpp SoundSegment_Simple.cpp"
    )
    
    if returncode != 0:
        print(f"❌ Compilation failed!")
        print(f"stderr: {stderr}")
        return False
    
    print("✅ C++ compilation successful")
    
    # Clean up
    if os.path.exists("test_compile"):
        os.remove("test_compile")
    
    return True

def test_basic_functionality():
    """Test basic functionality by running the demo."""
    print("Testing basic functionality...")
    
    # Compile the demo
    returncode, stdout, stderr = run_command(
        "g++ -std=c++11 -o test_demo demo_simple.cpp SoundSegment_Simple.cpp"
    )
    
    if returncode != 0:
        print(f"❌ Demo compilation failed!")
        return False
    
    # Run the demo
    returncode, stdout, stderr = run_command("./test_demo")
    
    if returncode != 0:
        print(f"❌ Demo execution failed!")
        return False
    
    # Check for expected output
    expected_phrases = [
        "Audio Editor C++ - Simple Demo",
        "Track length after writing 100 samples: 100",
        "All tests completed successfully"
    ]
    
    for phrase in expected_phrases:
        if phrase not in stdout:
            print(f"❌ Expected phrase not found: '{phrase}'")
            return False
    
    print("✅ Basic functionality test passed")
    
    # Clean up
    if os.path.exists("test_demo"):
        os.remove("test_demo")
    
    return True

def main():
    """Run all tests."""
    print("Audio Editor C++ - Python Test Suite")
    print("==========================================\n")
    
    tests = [
        ("Compilation", test_compilation),
        ("Basic Functionality", test_basic_functionality),
    ]
    
    passed = 0
    total = len(tests)
    
    for test_name, test_func in tests:
        print(f"\n Running {test_name} test...")
        try:
            if test_func():
                passed += 1
            else:
                print(f"❌ {test_name} test failed")
        except Exception as e:
            print(f"❌ {test_name} test failed with exception: {e}")
    
    print(f"\nTest Results: {passed}/{total} tests passed")
    
    if passed == total:
        print("All tests passed!")
        return 0
    else:
        print("Some tests failed")
        return 1

if __name__ == "__main__":
    sys.exit(main()) 