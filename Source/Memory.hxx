#pragma once

#include <memory_resource>
#include <cstddef>
#include <iostream>

static constexpr std::size_t ScratchBufferSize = 1024 * 1024 * 16;

class CScratchBuffer {
private:
    std::pmr::monotonic_buffer_resource *MemoryResource;
public:
    explicit CScratchBuffer(std::pmr::monotonic_buffer_resource *MemoryResource) : MemoryResource(MemoryResource) {
    }

    ~CScratchBuffer() {
        MemoryResource->release();
    }

    auto GetAllocator() {
        return MemoryResource;
    }

    template<typename T2>
    auto GetVector() {
        return std::pmr::vector<T2>(MemoryResource);
    }
};

class CNewDeleteResource final : public std::pmr::memory_resource {
    void *
    do_allocate(size_t Length, size_t Alignment) override {
        std::printf("Allocating %zu bytes through NewDeleteResource\n", Length);
        return ::operator new(Length, std::align_val_t(Alignment));
    }

    void
    do_deallocate(void *Pointer, size_t Length, size_t Alignment) noexcept
    override {
        std::printf("Deallocating %zu bytes through NewDeleteResource\n", Length);
        ::operator delete(Pointer, Length, std::align_val_t(Alignment));
    }

    [[nodiscard]] bool
    do_is_equal(const memory_resource &Other) const noexcept override { return &Other == this; }
};

namespace Memory {
    void Init();

    CScratchBuffer GetScratchBuffer();

    void Shutdown();
}