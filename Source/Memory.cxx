#include "Memory.hxx"
#include <memory_resource>

namespace Memory
{
    bool bMemorySystemInitialized = false;
    CNewDeleteResource* BaseResource{};
    std::pmr::monotonic_buffer_resource* ScratchBufferResource{};
    void* ScratchBufferData{};

    void Init()
    {
        if (bMemorySystemInitialized)
        {
            return;
        }

        BaseResource = new CNewDeleteResource();
        std::pmr::set_default_resource(std::pmr::null_memory_resource());
        // std::pmr::set_default_resource(BaseResource);

        ScratchBufferData = std::calloc(ScratchBufferSize, 1);
        ScratchBufferResource = new std::pmr::monotonic_buffer_resource(ScratchBufferData, ScratchBufferSize);

        bMemorySystemInitialized = true;
    }

    CScratchBuffer GetScratchBuffer()
    {
        return CScratchBuffer(ScratchBufferResource);
    }

    void Shutdown()
    {
        if (!bMemorySystemInitialized)
        {
            return;
        }

        ScratchBufferResource->release();
        std::pmr::set_default_resource(std::pmr::null_memory_resource());

        delete ScratchBufferResource;
        delete BaseResource;
    }
}

#if 0 // EQUINOX_REACH_DEVELOPMENT

#include <iostream>

void* operator new(std::size_t sz)
{
    std::printf("1) new(size_t), size = %zu\n", sz);
    if (sz == 0)
        ++sz;

    if (void* ptr = std::malloc(sz))
        return ptr;

    //    throw std::bad_alloc{};
}

void* operator new[](std::size_t sz)
{
    std::printf("2) new[](size_t), size = %zu\n", sz);
    if (sz == 0)
        ++sz;

    if (void* ptr = std::malloc(sz))
        return ptr;

    //    throw std::bad_alloc{};
}

void operator delete(void* ptr) noexcept
{
    std::puts("3) delete(void*)");
    std::free(ptr);
}

void operator delete(void* ptr, std::size_t size) noexcept
{
    std::printf("4) delete(void*, size_t), size = %zu\n", size);
    std::free(ptr);
}

void operator delete[](void* ptr) noexcept
{
    std::puts("5) delete[](void* ptr)");
    std::free(ptr);
}

void operator delete[](void* ptr, std::size_t size) noexcept
{
    std::printf("6) delete[](void*, size_t), size = %zu\n", size);
    std::free(ptr);
}

#endif
