#include "../include/lob/SnapshotBuffer.h"

SnapshotBuffer::SnapshotBuffer(): last_snapshot{}, seq{0} {}

void SnapshotBuffer::push(const BookSnapshot& snap) {
    seq.fetch_add(1, std::memory_order_release);
    last_snapshot = snap;
    seq.fetch_add(1, std::memory_order_release);
}

BookSnapshot SnapshotBuffer::pull() {
    uint64_t current_seq;
    BookSnapshot snap;

    do {
        current_seq = seq.load(std::memory_order_acquire);
        snap = last_snapshot;
    } while (current_seq % 2 != 0);
    
    return snap;
}

