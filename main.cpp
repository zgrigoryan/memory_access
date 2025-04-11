#include <iostream>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cassert>      
#include <random>
#include <thread>
#include <vector>
#include <algorithm>
#include <numeric>
#include <fstream>

// Architecture-specific includes
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
    #include <immintrin.h>  // AVX for x86/x64
#elif defined(__aarch64__) || defined(__arm64__)
    #include <arm_neon.h>  // Neon for ARM
#endif

// Function to sum aligned elements
template <typename T>
void sum_aligned(const T* data, size_t size, double& result, double& time_taken) {
    auto start = std::chrono::high_resolution_clock::now();
    
    result = 0.0;
    
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
    // AVX for x86/x64
    __m256d sum_vec = _mm256_setzero_pd();
    size_t i = 0;
    for (i = 0; i < size / 4 * 4; i += 4) {
        __m256d data_vec = _mm256_load_pd(&data[i]);  // Aligned load
        sum_vec = _mm256_add_pd(sum_vec, data_vec);
    }

    double sum[4];
    _mm256_store_pd(sum, sum_vec);
    result = sum[0] + sum[1] + sum[2] + sum[3];

#elif defined(__aarch64__) || defined(__arm64__)
    // Neon for ARM
    float64x2_t sum_vec = vdupq_n_f64(0.0); 
    size_t i = 0;
    for (i = 0; i < size / 2 * 2; i += 2) {
        float64x2_t data_vec = vld1q_f64(&data[i]);  // Load 2 doubles (aligned or unaligned)
        sum_vec = vaddq_f64(sum_vec, data_vec); 
    }

    double sum[2];
    vst1q_f64(sum, sum_vec); 
    result = sum[0] + sum[1];
#endif

    // Handle remaining elements
    for (size_t i = size / 2 * 2; i < size; i++) {
        result += data[i];
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    time_taken = duration.count();
}

// Function to sum unaligned elements
template <typename T>
void sum_unaligned(const T* data, size_t size, double& result, double& time_taken) {
    auto start = std::chrono::high_resolution_clock::now();
    
    result = 0.0;

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
    // AVX for x86/x64
    __m256d sum_vec = _mm256_setzero_pd();
    size_t i = 0;
    for (i = 0; i < size / 4 * 4; i += 4) {
        __m256d data_vec = _mm256_loadu_pd(&data[i]);  // Unaligned load
        sum_vec = _mm256_add_pd(sum_vec, data_vec);
    }

    double sum[4];
    _mm256_store_pd(sum, sum_vec);
    result = sum[0] + sum[1] + sum[2] + sum[3];

#elif defined(__aarch64__) || defined(__arm64__)
    // Neon for ARM
    float64x2_t sum_vec = vdupq_n_f64(0.0);  
    size_t i = 0;
    for (i = 0; i < size / 2 * 2; i += 2) {
        float64x2_t data_vec = vld1q_f64(&data[i]);  
        sum_vec = vaddq_f64(sum_vec, data_vec);  
    }

    double sum[2];
    vst1q_f64(sum, sum_vec); 
    result = sum[0] + sum[1];
#endif

    // Handle remaining elements
    for (size_t i = size / 2 * 2; i < size; i++) {
        result += data[i];
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    time_taken = duration.count();
}

// Template for measuring performance and writing to CSV
void measure_performance_and_write_csv(double* data, size_t size, const std::string& filename) {
    std::ofstream file;
    file.open(filename, std::ios::out);
    file << "Run,AlignedSum,AlignedTime,UnalignedSum,UnalignedTime\n";

    for (int run = 1; run <= 100; ++run) {
        double aligned_result, unaligned_result;
        double aligned_time, unaligned_time;

        // Measure aligned sum
        sum_aligned(data, size, aligned_result, aligned_time);

        // Measure unaligned sum
        double* unaligned_data = data + 1;  // Misalign the data
        sum_unaligned(unaligned_data, size, unaligned_result, unaligned_time);

        // Write results to CSV
        file << run << "," << aligned_result << "," << aligned_time << "," << unaligned_result << "," << unaligned_time << "\n";
    }

    file.close();
}

int main() {
    const size_t size = 1000000;
    std::vector<double> data(size);

    // Use random values to populate the array
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    std::generate(data.begin(), data.end(), [&]() { return dis(gen); });

    // Run the performance measurements and write results to CSV
    const std::string filename = "results.csv";
    measure_performance_and_write_csv(data.data(), size, filename);

    std::cout << "Results written to " << filename << std::endl;
    return 0;
}
