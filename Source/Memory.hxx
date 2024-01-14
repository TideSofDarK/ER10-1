#pragma once

#include <memory>
#include <cstring>
#include <mutex>
#include <memory_resource>
#include <vector>
#include <cstddef>
#include <array>
#include "Log.hxx"

static constexpr std::size_t HeapSize = 1024 * 1024 * 12;

template <size_t Size>
class TInlineResource : public std::pmr::memory_resource
{
private:
    struct SAllocationHeader
    {
        std::size_t Length{};
        SAllocationHeader* PreviousBlock{};
        SAllocationHeader* NextBlock{};
    };

    alignas(alignof(std::max_align_t))
        std::array<std::byte, Size> Buffer{};
    SAllocationHeader* FirstAllocation{};
    std::pmr::memory_resource* Upstream = std::pmr::new_delete_resource();

    void DestroyBlock(SAllocationHeader* AllocationHeader)
    {
        if (AllocationHeader->PreviousBlock == nullptr)
        {
            FirstAllocation = AllocationHeader->NextBlock;
        }
        else if (AllocationHeader->PreviousBlock != nullptr)
        {
            AllocationHeader->PreviousBlock->NextBlock = AllocationHeader->NextBlock;
        }
        if (AllocationHeader->NextBlock != nullptr)
        {
            AllocationHeader->NextBlock->PreviousBlock = AllocationHeader->PreviousBlock;
        }
    }

public:
    explicit TInlineResource(std::pmr::memory_resource* up = std::pmr::new_delete_resource())
        : Upstream(up)
    {
    }

#ifdef EQUINOX_REACH_DEVELOPMENT
    size_t NumberOfBlocks()
    {
        if (FirstAllocation == nullptr)
        {
            return 0;
        }

        size_t Num = 0;

        SAllocationHeader* AllocationHeader = FirstAllocation;
        while (AllocationHeader != nullptr)
        {
            Num++;
            AllocationHeader = AllocationHeader->NextBlock;
        }

        return Num;
    }
#endif

    inline void* do_allocate(size_t bytes, size_t alignment) override
    {
        std::unique_lock Lock{ Mutex };
        return Allocate(nullptr, bytes, alignment);
    }

    inline void do_deallocate(void* ptr, size_t bytes, size_t alignment) override
    {
        std::unique_lock Lock{ Mutex };
        Allocate(ptr, 0);
    }

    inline void* do_reallocate(void* ptr, size_t bytes)
    {
        std::unique_lock Lock{ Mutex };
        return Allocate(ptr, bytes);
    }

    std::mutex Mutex;

    inline void* Allocate(void* SrcPtr, size_t Bytes, const size_t Align = alignof(max_align_t))
    {
        std::size_t ReallocBytes{};
        if (SrcPtr != nullptr)
        {
            auto BytePtr = static_cast<std::byte*>(SrcPtr);
            auto AllocationHeader = reinterpret_cast<SAllocationHeader*>(BytePtr - sizeof(SAllocationHeader));

            /* Freeing. */
            if (Bytes == 0)
            {

                /* Check if the pointer is within our range. */
                if (BytePtr < Buffer.data() || BytePtr >= Buffer.data() + Buffer.size())
                {
                    Log::Memory("Freeing %zu bytes from upstream at %p.", AllocationHeader->Length, AllocationHeader);
                    Upstream->deallocate(AllocationHeader, AllocationHeader->Length + sizeof(SAllocationHeader), Align);
                    return nullptr;
                }

                DestroyBlock(AllocationHeader);

                Log::Memory("Freeing %zu bytes at %p.", AllocationHeader->Length, AllocationHeader);

                return nullptr;
            }

            if (AllocationHeader->NextBlock != nullptr)
            {
                if (BytePtr + sizeof(SAllocationHeader) + Bytes >= reinterpret_cast<std::byte*>(AllocationHeader->NextBlock))
                {
                    Log::Memory("Pending reallocation of %zu bytes into %zu at %p.", AllocationHeader->Length, Bytes, AllocationHeader);
                    /* Can't go beyond NextBlock! */
                    ReallocBytes = AllocationHeader->Length;
                    DestroyBlock(AllocationHeader);
                }
                else
                {
                    Log::Memory("Reallocating %zu bytes at %p.", Bytes, AllocationHeader);
                    AllocationHeader->Length = Bytes;
                    return SrcPtr;
                }
            }
            else
            {
                if (BytePtr + sizeof(SAllocationHeader) + Bytes >= Buffer.data() + Buffer.size())
                {
                    Log::Memory("Pending reallocation of %zu bytes into %zu at %p.", AllocationHeader->Length, Bytes, AllocationHeader);
                    /* Can't go beyond end of the buffer! */
                    ReallocBytes = AllocationHeader->Length;
                    DestroyBlock(AllocationHeader);
                }
                else
                {
                    Log::Memory("Reallocating %zu bytes at %p.", Bytes, AllocationHeader);
                    AllocationHeader->Length = Bytes;
                    return SrcPtr;
                }
            }
        }
        else if (Bytes == 0)
        {
            return nullptr;
        }

        std::size_t FinalAllocationSize = Bytes + sizeof(SAllocationHeader);

        std::byte* NewPtr{};
        SAllocationHeader* NewPreviousBlock = nullptr;
        SAllocationHeader* NewNextBlock = nullptr;

        if (FinalAllocationSize > Buffer.size())
        {
            NewPtr = static_cast<std::byte*>(Upstream->allocate(FinalAllocationSize, Align));
            Log::Memory("Allocating %zu bytes from upstream at %p.", Bytes, NewPtr);
        }
        else
        {
            if (FirstAllocation == nullptr)
            {
                NewPtr = Buffer.data();
                FirstAllocation = reinterpret_cast<SAllocationHeader*>(NewPtr);
                Log::Memory("Allocating %zu bytes at %p.", Bytes, NewPtr);
            }
            else
            {
                SAllocationHeader* AllocationHeader = FirstAllocation;
                while (AllocationHeader != nullptr)
                {
                    auto DataAfterHeader = reinterpret_cast<std::byte*>(AllocationHeader) + AllocationHeader->Length + sizeof(SAllocationHeader);

                    /* See if there is free space after the header. */
                    if (AllocationHeader->NextBlock != nullptr)
                    {
                        auto NextBlockData = reinterpret_cast<std::byte*>(AllocationHeader->NextBlock) + sizeof(SAllocationHeader);
                        if (DataAfterHeader + FinalAllocationSize < NextBlockData)
                        {
                            /* We can fit an allocation between two. */
                            NewPtr = DataAfterHeader;
                            NewPreviousBlock = AllocationHeader;
                            NewNextBlock = AllocationHeader->NextBlock;
                            Log::Memory("Allocating %zu bytes at %p.", Bytes, NewPtr);
                            break;
                        }
                        else
                        {
                            AllocationHeader = AllocationHeader->NextBlock;
                            continue;
                        }
                    }
                    else
                    {
                        /* No next block; just check if it fits. */
                        if (DataAfterHeader + FinalAllocationSize >= Buffer.data() + Buffer.size())
                        {
                            NewPtr = static_cast<std::byte*>(Upstream->allocate(FinalAllocationSize, Align));
                            Log::Memory("Allocating %zu bytes from upstream at %p.", Bytes, NewPtr);
                            break;
                        }
                        else
                        {
                            NewPtr = DataAfterHeader;
                            NewPreviousBlock = AllocationHeader;
                            NewNextBlock = AllocationHeader->NextBlock;
                            Log::Memory("Allocating %zu bytes at %p.", Bytes, NewPtr);
                            break;
                        }
                    }
                }
            }
        }

        if (NewPtr == nullptr)
        {
            Log::Memory("Failed to allocate %zu bytes.", Bytes);
            exit(1);
        }

        auto NewAllocationHeader = reinterpret_cast<SAllocationHeader*>(NewPtr);
        NewAllocationHeader->Length = Bytes;
        NewAllocationHeader->NextBlock = NewNextBlock;
        NewAllocationHeader->PreviousBlock = NewPreviousBlock;

        if (NewNextBlock != nullptr)
        {
            NewNextBlock->PreviousBlock = NewAllocationHeader;
        }

        if (NewPreviousBlock != nullptr)
        {
            NewPreviousBlock->NextBlock = NewAllocationHeader;
        }

        auto ResultPtr = NewPtr + sizeof(SAllocationHeader);

        if (ReallocBytes != 0)
        {
            Log::Memory("Copying %zu bytes to %p.", std::min(ReallocBytes, Bytes), ResultPtr);
            std::memcpy(ResultPtr, SrcPtr, std::min(ReallocBytes, Bytes));
        }

        return ResultPtr;
    }

    [[nodiscard]] inline bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override
    {
        return this == &other;
    }
};

class CMemory final
{
protected:
    class CTopmostResource final : public std::pmr::memory_resource
    {
        void*
        do_allocate(size_t Bytes, size_t Align) override
        {
            Log::Memory("Allocating %d bytes through topmost resource.", Bytes);
            return ::operator new(Bytes, std::align_val_t(Align));
        }

        void
        do_deallocate(void* Pointer, size_t Bytes, size_t Align) noexcept
            override
        {
            Log::Memory("Freeing %d bytes through topmost resource.", Bytes);
            ::operator delete(Pointer);
        }

        [[nodiscard]] inline bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override
        {
            return this == &other;
        }
    };

    CTopmostResource TopmostResource;
    TInlineResource<HeapSize> InlineResource;
    std::pmr::synchronized_pool_resource PoolResource;

    static CMemory& Get();

public:
    explicit CMemory()
        : InlineResource(
#ifdef EQUINOX_REACH_DEVELOPMENT
            std::pmr::null_memory_resource())
#else
            &this->TopmostResource)
#endif
        , PoolResource(std::pmr::pool_options{ .max_blocks_per_chunk = 0, .largest_required_pool_block = 0 }, &InlineResource)
    {
        Log::Memory("Initiated heap and pool resources, total size: %zu bytes.", HeapSize);
    };

    inline static void* Malloc(size_t Bytes)
    {
        void* Ptr = Get().InlineResource.do_allocate(Bytes, alignof(std::max_align_t));
        return Ptr;
    }

    inline static void* Calloc(size_t Num, size_t Bytes)
    {
        void* Ptr = Get().InlineResource.do_allocate(Num * Bytes, alignof(std::max_align_t));
        std::memset(Ptr, 0, Bytes * Num);
        return Ptr;
    }

    inline static void* Realloc(void* Ptr, size_t Bytes)
    {
        return Get().InlineResource.do_reallocate(Ptr, Bytes);
    }

    inline static void Free(void* Ptr)
    {
        Get().InlineResource.do_deallocate(Ptr, 0, alignof(std::max_align_t));
    }

    template <typename T>
    inline static std::shared_ptr<T> MakeShared()
    {
        return std::allocate_shared<T, std::pmr::polymorphic_allocator<T>>(&Get().InlineResource);
    }

    template <typename T>
    inline static auto GetVector()
    {
        return std::pmr::vector<T>(&Get().PoolResource);
    }

#ifdef EQUINOX_REACH_DEVELOPMENT
    static std::size_t NumberOfBlocks()
    {
        return Get().InlineResource.NumberOfBlocks();
    }
#endif
};
