#!/usr/bin/env python3
"""
Benchmark and stress testing script for the C++ Audio Editor.
"""

import time
import numpy as np
import matplotlib.pyplot as plt
from test_audio_editor import PythonAudioEditor
import random
import gc
import psutil
import os

class AudioEditorBenchmark:
    """Benchmark suite for audio editor performance testing"""
    
    def __init__(self):
        self.results = {}
    
    def time_function(self, func, *args, **kwargs):
        """Time a function execution"""
        start_time = time.perf_counter()
        result = func(*args, **kwargs)
        end_time = time.perf_counter()
        return result, end_time - start_time
    
    def measure_memory_usage(self):
        """Get current memory usage in MB"""
        process = psutil.Process(os.getpid())
        return process.memory_info().rss / 1024 / 1024
    
    def benchmark_write_operations(self, sizes):
        """Benchmark write operations with different data sizes"""
        print("Benchmarking write operations...")
        results = []
        
        for size in sizes:
            data = list(range(size))
            editor = PythonAudioEditor()
            
            _, duration = self.time_function(editor.write, data, 0)
            results.append((size, duration))
            print(f"  Size {size}: {duration:.4f}s")
        
        self.results['write_operations'] = results
        return results
    
    def benchmark_read_operations(self, sizes):
        """Benchmark read operations with different data sizes"""
        print("Benchmarking read operations...")
        results = []
        
        for size in sizes:
            data = list(range(size))
            editor = PythonAudioEditor()
            editor.write(data, 0)
            
            _, duration = self.time_function(editor.read, 0, size)
            results.append((size, duration))
            print(f"  Size {size}: {duration:.4f}s")
        
        self.results['read_operations'] = results
        return results
    
    def benchmark_delete_operations(self, sizes):
        """Benchmark delete operations with different data sizes"""
        print("Benchmarking delete operations...")
        results = []
        
        for size in sizes:
            data = list(range(size * 2))  # Create larger dataset
            editor = PythonAudioEditor()
            editor.write(data, 0)
            
            # Delete half of the data
            delete_size = size
            _, duration = self.time_function(editor.delete_range, size // 2, delete_size)
            results.append((size, duration))
            print(f"  Delete size {delete_size}: {duration:.4f}s")
        
        self.results['delete_operations'] = results
        return results
    
    def benchmark_insert_operations(self, sizes):
        """Benchmark insert operations with different data sizes"""
        print("Benchmarking insert operations...")
        results = []
        
        for size in sizes:
            # Create source and destination data
            src_data = list(range(size))
            dest_data = list(range(1000, 1000 + size))
            
            dest_editor = PythonAudioEditor()
            dest_editor.write(dest_data, 0)
            
            _, duration = self.time_function(dest_editor.insert, src_data, size // 2, 0, size)
            results.append((size, duration))
            print(f"  Insert size {size}: {duration:.4f}s")
        
        self.results['insert_operations'] = results
        return results
    
    def benchmark_correlation(self, target_sizes, ad_sizes):
        """Benchmark advertisement identification with different sizes"""
        print("Benchmarking advertisement identification...")
        results = []
        
        for target_size in target_sizes:
            for ad_size in ad_sizes:
                # Create target with random data
                target_data = [random.randint(-1000, 1000) for _ in range(target_size)]
                target_editor = PythonAudioEditor()
                target_editor.write(target_data, 0)
                
                # Create ad pattern
                ad_pattern = [random.randint(-1000, 1000) for _ in range(ad_size)]
                
                _, duration = self.time_function(target_editor.identify_ads, ad_pattern)
                results.append((target_size, ad_size, duration))
                print(f"  Target {target_size}, Ad {ad_size}: {duration:.4f}s")
        
        self.results['correlation_operations'] = results
        return results
    
    def stress_test_memory_usage(self, num_tracks=50, track_size=10000):
        """Stress test memory usage with multiple tracks"""
        print(f"Stress testing with {num_tracks} tracks of size {track_size}...")
        
        initial_memory = self.measure_memory_usage()
        print(f"Initial memory usage: {initial_memory:.2f} MB")
        
        tracks = []
        memory_usage = [initial_memory]
        
        for i in range(num_tracks):
            data = [random.randint(-32768, 32767) for _ in range(track_size)]
            editor = PythonAudioEditor()
            editor.write(data, 0)
            tracks.append(editor)
            
            current_memory = self.measure_memory_usage()
            memory_usage.append(current_memory)
            
            if i % 10 == 9:
                print(f"  Created {i+1} tracks, memory: {current_memory:.2f} MB")
        
        # Perform operations on tracks
        print("Performing operations on all tracks...")
        for i, track in enumerate(tracks):
            # Perform some operations
            track.read(0, min(100, track.length()))
            if track.length() > 100:
                track.delete_range(10, 50)
            
            if i % 10 == 9:
                current_memory = self.measure_memory_usage()
                memory_usage.append(current_memory)
                print(f"  Processed {i+1} tracks, memory: {current_memory:.2f} MB")
        
        # Clean up
        del tracks
        gc.collect()
        
        final_memory = self.measure_memory_usage()
        print(f"Final memory usage: {final_memory:.2f} MB")
        
        self.results['memory_usage'] = memory_usage
        return memory_usage
    
    def stress_test_large_operations(self):
        """Stress test with very large data operations"""
        print("Stress testing with large operations...")
        
        # Test with progressively larger data sizes
        sizes = [1000, 10000, 100000, 500000]
        
        for size in sizes:
            print(f"Testing with size {size}...")
            
            start_memory = self.measure_memory_usage()
            
            # Create large dataset
            data = list(range(size))
            editor = PythonAudioEditor()
            
            # Time write operation
            _, write_time = self.time_function(editor.write, data, 0)
            
            # Time read operation
            _, read_time = self.time_function(editor.read, 0, size)
            
            # Time large insert operation
            insert_data = list(range(1000))
            _, insert_time = self.time_function(editor.insert, insert_data, size // 2, 0, len(insert_data))
            
            end_memory = self.measure_memory_usage()
            
            print(f"  Write: {write_time:.4f}s, Read: {read_time:.4f}s, Insert: {insert_time:.4f}s")
            print(f"  Memory delta: {end_memory - start_memory:.2f} MB")
            
            del editor, data, insert_data
            gc.collect()
    
    def plot_results(self):
        """Plot benchmark results"""
        if not self.results:
            print("No benchmark results to plot")
            return
        
        fig, axes = plt.subplots(2, 2, figsize=(12, 10))
        fig.suptitle('Audio Editor Performance Benchmarks')
        
        # Plot write operations
        if 'write_operations' in self.results:
            sizes, times = zip(*self.results['write_operations'])
            axes[0, 0].plot(sizes, times, 'b-o')
            axes[0, 0].set_title('Write Operations')
            axes[0, 0].set_xlabel('Data Size')
            axes[0, 0].set_ylabel('Time (seconds)')
            axes[0, 0].grid(True)
        
        # Plot read operations
        if 'read_operations' in self.results:
            sizes, times = zip(*self.results['read_operations'])
            axes[0, 1].plot(sizes, times, 'g-o')
            axes[0, 1].set_title('Read Operations')
            axes[0, 1].set_xlabel('Data Size')
            axes[0, 1].set_ylabel('Time (seconds)')
            axes[0, 1].grid(True)
        
        # Plot delete operations
        if 'delete_operations' in self.results:
            sizes, times = zip(*self.results['delete_operations'])
            axes[1, 0].plot(sizes, times, 'r-o')
            axes[1, 0].set_title('Delete Operations')
            axes[1, 0].set_xlabel('Delete Size')
            axes[1, 0].set_ylabel('Time (seconds)')
            axes[1, 0].grid(True)
        
        # Plot memory usage
        if 'memory_usage' in self.results:
            memory = self.results['memory_usage']
            axes[1, 1].plot(range(len(memory)), memory, 'm-')
            axes[1, 1].set_title('Memory Usage Over Time')
            axes[1, 1].set_xlabel('Time Steps')
            axes[1, 1].set_ylabel('Memory (MB)')
            axes[1, 1].grid(True)
        
        plt.tight_layout()
        plt.savefig('benchmark_results.png', dpi=300, bbox_inches='tight')
        print("Benchmark results saved to benchmark_results.png")
    
    def run_comprehensive_benchmark(self):
        """Run all benchmarks"""
        print("Running comprehensive benchmark suite...")
        print("=" * 60)
        
        # Basic operation benchmarks
        sizes = [100, 500, 1000, 5000, 10000]
        
        self.benchmark_write_operations(sizes)
        print()
        
        self.benchmark_read_operations(sizes)
        print()
        
        self.benchmark_delete_operations(sizes)
        print()
        
        self.benchmark_insert_operations(sizes)
        print()
        
        # Correlation benchmark
        target_sizes = [1000, 5000, 10000]
        ad_sizes = [10, 50, 100]
        self.benchmark_correlation(target_sizes, ad_sizes)
        print()
        
        # Stress tests
        self.stress_test_memory_usage()
        print()
        
        self.stress_test_large_operations()
        print()
        
        # Generate plots
        try:
            self.plot_results()
        except ImportError:
            print("matplotlib not available, skipping plots")
        
        print("Benchmark suite completed!")

def main():
    """Main benchmark execution"""
    print("Audio Editor Benchmark Suite")
    print("=" * 40)
    
    # Check if required packages are available
    try:
        import matplotlib.pyplot as plt
        import psutil
    except ImportError as e:
        print(f"Warning: Some optional packages not available: {e}")
        print("Continuing with basic benchmarks...")
    
    benchmark = AudioEditorBenchmark()
    benchmark.run_comprehensive_benchmark()

if __name__ == '__main__':
    main() 