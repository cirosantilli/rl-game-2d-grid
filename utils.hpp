#ifndef UTILS_HPP
#define UTILS_HPP

#include <chrono>
#include <cstdio>

namespace utils {
    double get_secs(void);

    // FPS.
    extern const double FPS_GRANULARITY_S;
    extern double fps_last_time_s;
    extern unsigned int fps_nframes;
    void fps_init();
    void fps_update_and_print();
}

#endif
