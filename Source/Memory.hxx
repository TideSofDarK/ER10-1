#pragma once

#include <memory>
#include <cstring>
#include <mutex>
#include <memory_resource>
#include <vector>
#include <cstddef>
#include <array>
#include "Log.hxx"
#include "Utility.hxx"

static constexpr std::size_t HeapSize = 1024 * 1024 * 16;

template <size_t Size>
class TInlineResource final : public std::pmr::memory_resource
{
private:
    struct alignas(alignof(std::max_align_t)) SAllocationHeader
    {
        std::size_t Length{};
        SAllocationHeader* PreviousBlock{};
        SAllocationHeader* NextBlock{};
    };

    alignas(alignof(std::max_align_t))
        std::array<std::byte, Size> Buffer{};
    SAllocationHeader* FirstAllocation{};
    std::pmr::memory_resource* Upstream = std::pmr::new_delete_resource();
    std::mutex Mutex;

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
        Log::Memory("Creating inline resource, total size: %zu bytes, data(): %p", HeapSize, Buffer.data());
    }

    ~TInlineResource() final
    {
        Log::Memory("Destroying inline resource, NumberOfBlocks: %zu", NumberOfBlocks());
    };

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

    inline void* do_allocate(size_t bytes, size_t alignment) override
    {
        std::unique_lock Lock{ Mutex };
        return AllocateInline(nullptr, bytes, alignment);
    }

    inline void do_deallocate(void* ptr, size_t bytes, size_t alignment) override
    {
        std::unique_lock Lock{ Mutex };
        AllocateInline(ptr, 0);
    }

    inline void* do_reallocate(void* ptr, size_t bytes)
    {
        std::unique_lock Lock{ Mutex };
        return AllocateInline(ptr, bytes);
    }

    inline void* AllocateInline(void* SrcPtr, size_t Bytes, const size_t Alignment = alignof(std::max_align_t))
    {
        std::size_t BytesWithHeader = Bytes + sizeof(SAllocationHeader);
        std::size_t FinalAllocationSize = BytesWithHeader + (Alignment - 1);
        std::size_t ReallocBytes{};
        if (SrcPtr != nullptr)
        {
            auto BytePtr = static_cast<std::byte*>(SrcPtr);
            auto AllocationHeader = reinterpret_cast<SAllocationHeader*>(BytePtr) - 1;

            /* Freeing. */
            if (Bytes == 0)
            {
                /* Check if the pointer is within our range. */
                if (BytePtr < Buffer.data() || BytePtr >= Buffer.data() + Buffer.size())
                {
                    Log::Memory("Freeing from upstream at %p", BytePtr);
                    Upstream->deallocate(SrcPtr, Bytes, Alignment);
                    return nullptr;
                }

                DestroyBlock(AllocationHeader);

                Log::Memory("Freeing %zu bytes at %p", AllocationHeader->Length, BytePtr);

                return nullptr;
            }

            /* Reallocating. */
            if (AllocationHeader->NextBlock != nullptr)
            {
                if (BytePtr + BytesWithHeader >= reinterpret_cast<std::byte*>(AllocationHeader->NextBlock) || !Utility::IsAlignedPtr(SrcPtr, Alignment))
                {
                    Log::Memory("Pending reallocation of %zu bytes into %zu at %p", AllocationHeader->Length, Bytes, BytePtr);
                    /* Can't go beyond NextBlock! */
                    ReallocBytes = std::min(AllocationHeader->Length, Bytes);
                    DestroyBlock(AllocationHeader);
                }
                else
                {
                    Log::Memory("Reallocating %zu bytes at %p", Bytes, BytePtr);
                    AllocationHeader->Length = Bytes;
                    return SrcPtr;
                }
            }
            else
            {
                if (BytePtr + BytesWithHeader >= Buffer.data() + Buffer.size() || !Utility::IsAlignedPtr(SrcPtr, Alignment))
                {
                    Log::Memory("Pending reallocation of %zu bytes into %zu at %p", AllocationHeader->Length, Bytes, BytePtr);
                    /* Can't go beyond end of the buffer! */
                    ReallocBytes = std::min(AllocationHeader->Length, Bytes);
                    DestroyBlock(AllocationHeader);
                }
                else
                {
                    Log::Memory("Reallocating %zu bytes at %p", Bytes, BytePtr);
                    AllocationHeader->Length = Bytes;
                    return SrcPtr;
                }
            }
        }
        else if (Bytes == 0)
        {
            return nullptr;
        }

        std::byte* NewPtr{};
        SAllocationHeader* NewPreviousBlock = nullptr;
        SAllocationHeader* NewNextBlock = nullptr;

        if (FinalAllocationSize > Buffer.size())
        {
            NewPtr = static_cast<std::byte*>(Upstream->allocate(Bytes, Alignment));

            Log::Memory("Allocating %zu bytes from upstream at %p", Bytes, NewPtr);

            if (ReallocBytes != 0)
            {
                Log::Memory("Copying %zu bytes to %p", std::min(ReallocBytes, Bytes), NewPtr);
                std::memcpy(NewPtr, SrcPtr, std::min(ReallocBytes, Bytes));
            }

            return NewPtr;
        }
        else
        {
            if (FirstAllocation == nullptr)
            {
                NewPtr = Buffer.data();
            }
            else
            {
                if (reinterpret_cast<std::byte*>(FirstAllocation) > Buffer.data() && reinterpret_cast<std::byte*>(FirstAllocation) - Buffer.data() >= BytesWithHeader)
                {
                    NewPtr = Buffer.data();
                    NewNextBlock = FirstAllocation;
                    FirstAllocation = nullptr;
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
                            auto NextBlockData = reinterpret_cast<std::byte*>(AllocationHeader->NextBlock);
                            if (DataAfterHeader + FinalAllocationSize < NextBlockData)
                            {
                                /* We can fit an allocation between two. */
                                NewPtr = DataAfterHeader;
                                NewPreviousBlock = AllocationHeader;
                                NewNextBlock = AllocationHeader->NextBlock;
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
                                NewPtr = static_cast<std::byte*>(Upstream->allocate(Bytes, Alignment));

                                Log::Memory("Allocating %zu bytes from upstream at %p", Bytes, NewPtr);

                                if (ReallocBytes != 0)
                                {
                                    Log::Memory("Copying %zu bytes to %p", std::min(ReallocBytes, Bytes), NewPtr);
                                    std::memcpy(NewPtr, SrcPtr, std::min(ReallocBytes, Bytes));
                                }

                                return NewPtr;
                            }
                            else
                            {
                                NewPtr = DataAfterHeader;
                                NewPreviousBlock = AllocationHeader;
                                NewNextBlock = AllocationHeader->NextBlock;
                                break;
                            }
                        }
                    }
                }
            }
        }

        if (NewPtr == nullptr)
        {
            Log::Memory("Failed to allocate %zu bytes", Bytes);
            exit(1);
        }
        else
        {
            /* Shift pointer to point to allocated data. */
            NewPtr = static_cast<std::byte*>(Utility::AlignPtr(NewPtr + sizeof(SAllocationHeader), Alignment));
#ifdef EQUINOX_REACH_DEVELOPMENT
            if (ReallocBytes == 0)
            {
                Log::Memory("Allocating %zu bytes at %p", Bytes, NewPtr);
            }
#endif
        }

        auto NewAllocationHeader = reinterpret_cast<SAllocationHeader*>(NewPtr) - 1;
        NewAllocationHeader->Length = Bytes;
        NewAllocationHeader->NextBlock = NewNextBlock;
        NewAllocationHeader->PreviousBlock = NewPreviousBlock;

        if (FirstAllocation == nullptr)
        {
            FirstAllocation = NewAllocationHeader;
        }

        if (NewNextBlock != nullptr)
        {
            NewNextBlock->PreviousBlock = NewAllocationHeader;
        }

        if (NewPreviousBlock != nullptr)
        {
            NewPreviousBlock->NextBlock = NewAllocationHeader;
        }

        if (ReallocBytes != 0)
        {
            Log::Memory("Copying %zu bytes to %p", std::min(ReallocBytes, Bytes), NewPtr);
            std::memcpy(NewPtr, SrcPtr, std::min(ReallocBytes, Bytes));
        }

        return NewPtr;
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
            Log::Memory("Allocating %d bytes through topmost resource", Bytes);
            return ::operator new(Bytes, std::align_val_t(Align));
        }

        void
        do_deallocate(void* Pointer, size_t Bytes, size_t Align) noexcept
            override
        {
            Log::Memory("Freeing %d bytes through topmost resource", Bytes);
            ::operator delete(Pointer, Bytes, std::align_val_t(Align));
        }

        [[nodiscard]] inline bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override
        {
            return this == &other;
        }
    };

    TInlineResource<HeapSize> InlineResource;
    CTopmostResource TopmostResource;
    std::pmr::synchronized_pool_resource PoolResource;

    explicit CMemory()
        : InlineResource(&this->TopmostResource)
        , PoolResource(std::pmr::pool_options{ .max_blocks_per_chunk = 0, .largest_required_pool_block = 0 }, &InlineResource){};

    static CMemory& Instance()
    {
        static CMemory Instance;
        return Instance;
    }

    static CMemory& Get()
    {
        static std::once_flag Flag;
        std::call_once(Flag, [] { Instance(); });
        return Instance();
    }

public:
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

    static std::size_t NumberOfBlocks()
    {
        return Get().InlineResource.NumberOfBlocks();
    }
};
