#include "../include/lob/SnapshotBuffer.h"

SnapshotBuffer::SnapshotBuffer(): buffer{} {}

void SnapshotBuffer::push(BookSnapshot snap) {
    std::lock_guard<std::mutex> lock(mutex_lock);
    buffer.push(snap);
}

BookSnapshot SnapshotBuffer::pull() {
    std::lock_guard<std::mutex> lock(mutex_lock);
    auto snap = buffer.front();
    buffer.pop();
    return snap;
}

