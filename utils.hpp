#ifndef UTILS_HPP
#define UTILS_HPP

#include <chrono>
#include <cstdio>

namespace utils {
    double get_secs(void);

    namespace fps {
        double get();
        void init();
        void print();
        void update();
    }
}

#endif
