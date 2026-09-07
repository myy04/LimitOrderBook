#ifndef SNAPSHOT_BUFFER_H
#define SNAPSHOT_BUFFER_H

#include "./Types.h"

#include <queue>
#include <mutex>

class SnapshotBuffer {
public:
    explicit SnapshotBuffer();
    void push(const BookSnapshot& snapshot);

    BookSnapshot pull();
private:    
    BookSnapshot last_snapshot;
    std::atomic<uint64_t> seq;
};

#endif