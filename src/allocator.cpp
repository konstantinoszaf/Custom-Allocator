#include "allocator/allocator.h"
#include <cassert>
#include <cstdlib>

constexpr std::size_t MAX_CACHED_BLOCKS = 1024;
constexpr std::size_t ALIGNMENT = 32;

Allocator::Allocator() : activeMemory{}, readyToBeUsedMemory{} {}

Allocator::~Allocator() {
    for (const auto& [size, reusable_memory] : readyToBeUsedMemory) {
        for (const auto& meta : reusable_memory) {
            std::free(meta.memory);
        }
    }

    for (const auto& [ptr, meta] : activeMemory) {
        std::free(ptr);
    }
}

void* Allocator::allocate(std::size_t size) {
    if (size == 0) {
        return nullptr;
    }

    std::size_t bucket_size = align_to_bucket_size(size);
    auto it = readyToBeUsedMemory.find(bucket_size);

    if (it != readyToBeUsedMemory.end()) {
        auto& reusable_memory = it->second;
        assert(!reusable_memory.empty() && "Found size key, but vector was empty!");

        auto item = std::move(reusable_memory.back());
        reusable_memory.pop_back();

        if (reusable_memory.empty()) {
            // clean up the empty bucket
            readyToBeUsedMemory.erase(it);
        }

        item.state = MemoryState::IN_USE;
        void* memory = item.memory;
        activeMemory.emplace(memory, std::move(item));
        return memory;
    }

    // there is no reusable memory, we need to allocate new
    void* memory = std::malloc(bucket_size);
    if (!memory) {
        return nullptr;
    }

    activeMemory.emplace(memory, MemoryMetadata(memory, bucket_size, MemoryState::IN_USE));
    return memory;
}

void Allocator::deallocate(void* ptr) {
    if (!ptr) {
        return;
    }

    auto active_it = activeMemory.find(ptr);
    if (active_it == activeMemory.end()) {
        return;
    }

    auto meta = std::move(active_it->second);
    std::size_t size = meta.allocated_size;
    meta.state = MemoryState::INACTIVE;

    activeMemory.erase(active_it);

    auto& reusable_memory = readyToBeUsedMemory[size]; // creates an empty vector if size is not present in the map

    if (reusable_memory.size() > MAX_CACHED_BLOCKS) {
        std::free(ptr);
        return;
    }

    reusable_memory.push_back(std::move(meta));
    return;
}

std::size_t Allocator::align_to_bucket_size(std::size_t size) const {
    return (size + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1);
}

bool Allocator::is_allocated(void* ptr) const {
    if (!ptr) {
        return false;
    }

    return activeMemory.find(ptr) != activeMemory.end();
}