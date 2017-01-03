#include "utils.hpp"

namespace utils {
    double get_secs(void) {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count() / 1000000000.0;
    }

    namespace fps {
        bool new_ = false;
        const double GRANULARITY_S = 0.5;
        double last_time_s, fps;
        unsigned int nframes;

        double get() {
            return fps;
        }

        void init() {
            nframes = 0;
            last_time_s = get_secs();
        }

        void update() {
            double dt, current_time_s;
            current_time_s = get_secs();
            nframes++;
            dt = current_time_s - last_time_s;
            if (dt > GRANULARITY_S) {
                fps = nframes / dt;
                last_time_s = current_time_s;
                nframes = 0;
                new_ = true;
            }
        }

        void print() {
            if (new_) {
                std::printf("FPS = %f\n", utils::fps::get());
                new_ = false;
            }
        }
    }
}
