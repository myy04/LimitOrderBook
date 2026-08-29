#ifndef SNAPSHOT_BUFFER_H
#define SNAPSHOT_BUFFER_H

#include <queue>
#include "./Types.h"
#include <mutex>

class SnapshotBuffer {
public:
    explicit SnapshotBuffer();
    void push(BookSnapshot snapshot);

    BookSnapshot pull();
private:    
    std::queue<BookSnapshot> buffer;
    std::mutex mutex_lock;
};

#endif