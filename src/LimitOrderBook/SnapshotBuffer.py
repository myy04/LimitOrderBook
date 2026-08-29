from asyncio import QueueEmpty
import queue

from .DataTypes import BookSnapshot

class SnapshotBuffer:
    def __init__(self):
        self.buffer: queue.Queue[BookSnapshot] = queue.Queue()

    def pull(self) -> BookSnapshot:
        return self.buffer.get_nowait()

    def push(self, snapshot: BookSnapshot) -> None:
        self.buffer.put(snapshot)
