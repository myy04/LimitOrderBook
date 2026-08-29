#include "../include/lob/SnapshotBuffer.h"

SnapshotBuffer::SnapshotBuffer(): buffer{} {}

void SnapshotBuffer::push(BookSnapshot snap) {
    std::lock_guard<std::mutex> lock(mutex_lock);
    buffer.push(std::move(snap));
}

BookSnapshot SnapshotBuffer::pull() {
    std::lock_guard<std::mutex> lock(mutex_lock);
    if (buffer.empty()) return BookSnapshot();
    auto snap = buffer.front();
    buffer.pop();
    return snap;
}

