# Custom Allocator

A simple pool-style memory allocator built on top of malloc/free. It bypasses repeated OS calls by holding on to freed blocks and handing them back out when a matching size is requested again, instead of releasing them back to the OS immediately.

## How it works

There are two maps:

- `activeMemory`: tracks every pointer currently in use, keyed by the pointer itself. Used for O(1) lookup on deallocate and is_allocated.
- `readyToBeUsedMemory`: tracks freed blocks, keyed by their allocated size. Each entry is a vector of blocks of that size, acting as a small free list per size class.

When you call `allocate(size)`, the size is rounded up to the nearest multiple of 32 (see `align_to_bucket_size`). This buckets similar requests together (500, 512, 514 bytes all land in the same bucket) so the free list doesn't end up with a huge number of near-duplicate size keys.

If there's a free block already sitting in that bucket, it gets reused and moved into `activeMemory`. If not, a new block is allocated with malloc.

When you call `deallocate(ptr)`, the block is not freed back to the OS. It's moved out of `activeMemory` and into the free list bucket for its size, so a future `allocate` of the same size can reuse it. This is the core idea of the assignment: keep freed pointers around instead of handing them back to the OS.

There is a cap (`MAX_CACHED_BLOCKS`, currently 1024 per bucket) so the free list doesn't grow unbounded. Once a bucket hits the cap, further deallocations of that size are actually freed with `std::free`.

The destructor frees everything left in both maps, so no leaks on shutdown.

## Tradeoffs

**Not thread safe.** Both maps are accessed with no locking. This was a deliberate choice. The assignment was scoped to avoid over-engineering, so the assumption was made that the Allocator would be used from a single thread. If this needs to support concurrent allocate/deallocate/is_valid calls from multiple threads, some form of synchronization is needed.

**No TTL on cached blocks.** I considered a time-based eviction policy for the free list, where a block sitting unused for too long would get freed. I opted for the simpler hard cap (`MAX_CACHED_BLOCKS`) instead. A TTL adds a time dimension, timestamps on cached blocks and something to periodically check and evict them, which felt like unnecessary complexity for the scope of this assignment.

**No garbage collection.** Freeing memory is entirely manual, the caller has to call `deallocate`. Given the instruction to not over-engineer, I kept memory management manual and explicit.

## How to use

Build (release):

```
make build
```

This configures and builds the project with CMake in `build/`.

Build the driver (debug, includes the main executable):

```
make build-main
```

Run the driver:

```
make run-main
```

Clean the build directory:

```
make clean
```