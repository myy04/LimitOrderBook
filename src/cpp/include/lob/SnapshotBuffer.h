#ifndef SNAPSHOT_BUFFER_H
#define SNAPSHOT_BUFFER_H

#include <queue>
#include "./Types.h"
#include <mutex>

class SnapshotBuffer {
public:
    explicit SnapshotBuffer() = default;
    void push(BookSnapshot snapshot);

    BookSnapshot pull();
private:    
    BookSnapshot last_snapshot;
    std::mutex mutex_lock;
};

#endif