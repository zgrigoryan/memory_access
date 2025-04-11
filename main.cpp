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

template <typename T>
void sum_aligned(const T* data, size_t size) {
    double result = 0.0;
    
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
    // Neon for ARM (Fix for double-precision)
    float64x2_t sum_vec = vdupq_n_f64(0.0);  // Initialize Neon vector to zero (double precision)
    size_t i = 0;
    for (i = 0; i < size / 2 * 2; i += 2) {
        float64x2_t data_vec = vld1q_f64(&data[i]);  // Load 2 doubles (aligned or unaligned)
        sum_vec = vaddq_f64(sum_vec, data_vec);  // Add the two values (double precision)
    }

    double sum[2];
    vst1q_f64(sum, sum_vec);  // Store the result from the Neon vector
    result = sum[0] + sum[1];
#endif

    // Handle remaining elements
    for (size_t i = size / 2 * 2; i < size; i++) {
        result += data[i];
    }

    std::cout << "Aligned sum: " << result << std::endl;
}

template <typename T>
void sum_unaligned(const T* data, size_t size) {
    double result = 0.0;

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
    // Neon for ARM (Fix for double-precision)
    float64x2_t sum_vec = vdupq_n_f64(0.0);  // Initialize Neon vector to zero (double precision)
    size_t i = 0;
    for (i = 0; i < size / 2 * 2; i += 2) {
        float64x2_t data_vec = vld1q_f64(&data[i]);  // Load 2 doubles (unaligned)
        sum_vec = vaddq_f64(sum_vec, data_vec);  // Add the two values (double precision)
    }

    double sum[2];
    vst1q_f64(sum, sum_vec);  // Store the result from the Neon vector
    result = sum[0] + sum[1];
#endif

    // Handle remaining elements
    for (size_t i = size / 2 * 2; i < size; i++) {
        result += data[i];
    }

    std::cout << "Unaligned sum: " << result << std::endl;
}

// Template for measuring performance
template <typename Func>
void measure_performance(Func func, const double* data, size_t size) {
    auto start = std::chrono::high_resolution_clock::now();
    func(data, size);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << "Function took: " << duration.count() << " seconds." << std::endl;
}

int main() {
    const size_t size = 1000000;
    std::vector<double> data(size);

    // Use random values to populate the array
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    std::generate(data.begin(), data.end(), [&]() { return dis(gen); });

    // Aligned access: Allocate aligned memory
    double* aligned_data = nullptr;
    posix_memalign(reinterpret_cast<void**>(&aligned_data), 32, size * sizeof(double));
    std::memcpy(aligned_data, data.data(), size * sizeof(double));

    measure_performance([&](const double* data, size_t size) { sum_aligned(data, size); }, aligned_data, size);

    // Unaligned access: Introduce misalignment by adding offset
    double* unaligned_data = data.data() + 1;  

    measure_performance([&](const double* data, size_t size) { sum_unaligned(data, size); }, unaligned_data, size);

    free(aligned_data);  

    return 0;
}
