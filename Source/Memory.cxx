#include "Memory.hxx"

namespace Memory {
    long Allocations{};
    long Frees{};
    bool bMemorySystemInitialized = false;
    CNewDeleteResource *BaseResource{};
    std::pmr::monotonic_buffer_resource *ScratchBufferResource{};
    void *ScratchBufferData{};

    void Init() {
        if (bMemorySystemInitialized) {
            return;
        }

        BaseResource = new CNewDeleteResource();
        std::pmr::set_default_resource(BaseResource);

        ScratchBufferData = std::calloc(ScratchBufferSize, 1);
        ScratchBufferResource = new std::pmr::monotonic_buffer_resource(ScratchBufferData, ScratchBufferSize);

        bMemorySystemInitialized = true;
    }

    CScratchBuffer GetScratchBuffer() {
        return CScratchBuffer(ScratchBufferResource);
    }

    void Shutdown() {
        if (!bMemorySystemInitialized) {
            return;
        }

        ScratchBufferResource->release();
        std::pmr::set_default_resource(std::pmr::null_memory_resource());

        delete ScratchBufferResource;
        delete BaseResource;

        std::printf("Shutting down Memory; Allocation count: %ld, Free count: %ld\n", Allocations, Frees);
    }
}

extern "C" {
void *__real_malloc(size_t size); /** NOLINT */
void __real_free(void *ptr); /** NOLINT */

void *__wrap_malloc(size_t size) {
    Memory::Allocations++;
    void *ptr = __real_malloc(size);
    return ptr;
}

void __wrap_free(void *ptr) {
    Memory::Frees++;
    __real_free(ptr);
}
}