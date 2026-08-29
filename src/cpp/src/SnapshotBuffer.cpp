#include "../include/lob/SnapshotBuffer.h"

void SnapshotBuffer::push(BookSnapshot snap) {
    std::lock_guard<std::mutex> lock(mutex_lock);
    last_snapshot = std::move(snap);
}

BookSnapshot SnapshotBuffer::pull() {
    std::lock_guard<std::mutex> lock(mutex_lock);
    return last_snapshot;
}

