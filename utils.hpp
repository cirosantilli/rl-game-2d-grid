#ifndef UTILS_HPP
#define UTILS_HPP

#include <chrono>
#include <cstdio>

#define UTILS_CASE_COUT(x) \
    case Object::Type::x: \
        os << #x; \
    break;

#define UTILS_COMMAFY(X) X,

#define UTILS_ID(X) X

#define UTILS_DECLARE(TYPE, ID) TYPE ID;

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
