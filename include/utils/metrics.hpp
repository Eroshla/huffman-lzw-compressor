#ifndef METRICS_H
#define METRICS_H

#include <chrono>

namespace Metrics {

    inline std::chrono::high_resolution_clock::time_point start() {
        return std::chrono::high_resolution_clock::now();
    }

    inline double stop(const std::chrono::high_resolution_clock::time_point& start_time) {
        auto end_time = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(end_time - start_time).count();
    }

    inline double calculate_ratio(size_t compressed_size, size_t original_size) {
        if (original_size == 0) return 0.0;
        return (double)compressed_size / original_size;
    }

    inline double calculate_savings(size_t compressed_size, size_t original_size) {
        if (original_size == 0) return 0.0;
        return (1.0 - calculate_ratio(compressed_size, original_size)) * 100.0;
    }
}

#endif // METRICS_H
