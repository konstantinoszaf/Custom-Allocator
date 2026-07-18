#include "allocator/allocator.h"
#include <cassert>
#include <cstdio>
#include <iostream>

int main() {
    std::size_t buffer_size = 62;
    char* buffer = static_cast<char*>(Allocator::instance().allocate(buffer_size));
    std::snprintf(buffer, buffer_size, "%s", "hello from my custom allocator!");

    std::cout << "Buffer contents: " << buffer << "\n";
    buffer[0] = 'H';
    std::cout << "Updated buffer contents: " << buffer << "\n";

    assert(Allocator::instance().is_allocated(buffer) && "Pointer should be valid");
    std::cout   << "Is pointer valid: " << std::boolalpha
                << Allocator::instance().is_allocated(buffer) << '\n';

    Allocator::instance().deallocate(buffer);
    assert(!Allocator::instance().is_allocated(buffer) && "Pointer should not be valid");

    std::cout   << "After deallocation, Is pointer valid: "
                << std::boolalpha << Allocator::instance().is_allocated(buffer) << '\n';

    void* buffer2 = Allocator::instance().allocate(buffer_size);

    std::cout << "Original ptr: " << static_cast<void*>(buffer) << '\n';
    std::cout << "Recycled ptr: " << buffer2 << '\n';

    assert(buffer == buffer2 && "Pointer should be reused");

    return 0;
}