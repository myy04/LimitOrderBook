#ifndef CONFIG_H
#define CONFIG_H

#include <chrono>

namespace CONFIG {
    const std::chrono::seconds SNAPSHOT_PERIOD(1); 
    const size_t SNAPSHOT_DEPTH = 10;
}   

#endif