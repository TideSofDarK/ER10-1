#include "Memory.hxx"

namespace Memory {
    bool bMemorySystemInitialized = false;
    CNewDeleteResource *BaseResource{};
    std::pmr::monotonic_buffer_resource *ScratchBufferResource{};
    void *ScratchBufferData{};

    void Init() {
        if (bMemorySystemInitialized) {
            return;
        }

        BaseResource = new CNewDeleteResource();
        std::pmr::set_default_resource(std::pmr::null_memory_resource());
//        std::pmr::set_default_resource(BaseResource);

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
    }
}