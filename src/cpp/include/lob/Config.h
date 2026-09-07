#ifndef CONFIG_H
#define CONFIG_H

#include <chrono>

namespace CONFIG {
    const std::chrono::seconds SNAPSHOT_PERIOD(1); 
    const size_t SNAPSHOT_DEPTH = 10;   
    const float MIN_PRICE = 0.1;
    const float MAX_PRICE = 1e5;

    const size_t MIN_VOLUME = 1;
    const size_t MAX_VOLUME = 1e5;

    const float PRICE_TICK_SIZE = 0.1;

    const float EPS = 1e-9; 

    constexpr bool CAPTURE_SNAPSHOTS = false;
}   

#endif