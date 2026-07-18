#pragma once
#include <unordered_map>
#include <vector>

/**
 * Allocator is not thread safe. All calls to allocate, deallocate, and
 * is_valid must come from a single thread, or be externally synchronized.
 */
class Allocator {
public:
    static Allocator& instance() {
        static Allocator instance;
        return instance;
    }

    /**
     * Returns a block of at least `size` bytes, or nullptr if size == 0.
     * The caller is responsible for calling deallocate() on the returned
     * pointer exactly once. Failing to call deallocate() will leak the
     * block until the Allocator itself is destroyed.
     */
    void* allocate(std::size_t size);

    /**
     * Releases a pointer previously returned by allocate(). The block is
     * not necessarily freed back to the OS immediately, it may be cached
     * for reuse by a future allocate() call of the same size.
     * assing a pointer not currently owned by this allocator is a no-op.
     */
    void deallocate(void* ptr);
    bool is_allocated(void* ptr) const;

    Allocator(const Allocator&)             = delete;
    Allocator& operator=(const Allocator&)  = delete;
    Allocator(Allocator&&)                  = delete;
    Allocator& operator=(Allocator&&)       = delete;

private:
    Allocator();
    ~Allocator();

    /**
     * @brief Rounds a requested memory size up to the nearest multiple of ALIGNMENT.
     *
     * Uses a bitwise mask to snap the size to the next boundary.
     * (ALIGNMENT must strictly be a power of 2 for this formula to work).
     *
     * Groups similar allocation requests (e.g., 500, 512, 514 bytes)
     * into the same size bucket. This prevents fragmentation, drastically reducing
     * the number of unique keys in the readyToBeUsedMemory map.
     */
    std::size_t align_to_bucket_size(std::size_t size) const;

    /**
     * Not currently read anywhere in the implementation, both maps already
     * encode a block's state structurally, but kept since it makes each
     * MemoryMetadata instance self-describing on its own.
     */
    enum class MemoryState {
        IN_USE,
        INACTIVE
    };

    struct MemoryMetadata {
        void* memory;
        std::size_t allocated_size;
        MemoryState state;
        MemoryMetadata() : memory{nullptr}, allocated_size{0}, state{MemoryState::INACTIVE} {}
        MemoryMetadata(void *memory_, std::size_t allocated_size_, MemoryState state_)
        : memory{memory_}, allocated_size{allocated_size_}, state{state_} {}
    };

    std::unordered_map<void*, MemoryMetadata> activeMemory;
    std::unordered_map<std::size_t, std::vector<MemoryMetadata>> readyToBeUsedMemory;
};