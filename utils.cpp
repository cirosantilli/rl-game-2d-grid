#include "utils.hpp"

namespace utils {
    double get_secs(void) {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count() / 1000000000.0;
    }

    const double FPS_GRANULARITY_S = 0.5;
    double fps_last_time_s;
    unsigned int fps_nframes;
    void fps_init() {
        fps_nframes = 0;
        fps_last_time_s = get_secs();
    }
    void fps_update_and_print() {
        double dt, current_time_s;
        current_time_s = get_secs();
        fps_nframes++;
        dt = current_time_s - fps_last_time_s;
        if (dt > FPS_GRANULARITY_S) {
            std::printf("FPS = %f\n", fps_nframes / dt);
            fps_last_time_s = current_time_s;
            fps_nframes = 0;
        }
    }
}
