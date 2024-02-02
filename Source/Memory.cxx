#include "Memory.hxx"

#include "Log.hxx"
#include "Utility.hxx"

CInlineResource::CInlineResource(std::pmr::memory_resource* up)
    : Upstream(up)
{
    Log::Memory<ELogLevel::Critical>("Creating inline resource, total size: %zu bytes, data(): %p", HeapSize, Buffer.data());
}

CInlineResource::~CInlineResource()
{
    Log::Memory<ELogLevel::Critical>("Destroying inline resource, NumberOfBlocks: %zu", NumberOfBlocks());
}

void CInlineResource::DestroyBlock(SAllocationHeader* AllocationHeader)
{
    if (AllocationHeader->PreviousBlock == nullptr)
    {
        FirstAllocation = AllocationHeader->NextBlock;
    }
    else
    {
        AllocationHeader->PreviousBlock->NextBlock = AllocationHeader->NextBlock;
    }
    if (AllocationHeader->NextBlock != nullptr)
    {
        AllocationHeader->NextBlock->PreviousBlock = AllocationHeader->PreviousBlock;
    }
}

size_t CInlineResource::NumberOfBlocks()
{
    std::unique_lock Lock{ Mutex };
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

void* CInlineResource::do_allocate(size_t bytes, size_t alignment)
{
    std::unique_lock Lock{ Mutex };
    return AllocateInline(nullptr, bytes, alignment);
}

void CInlineResource::do_deallocate(void* ptr, size_t bytes, size_t alignment)
{
    std::unique_lock Lock{ Mutex };
    AllocateInline(ptr, 0);
}

void* CInlineResource::do_reallocate(void* ptr, size_t bytes)
{
    std::unique_lock Lock{ Mutex };
    return AllocateInline(ptr, bytes);
}

void* CInlineResource::AllocateInline(void* SrcPtr, size_t Bytes, const size_t Alignment)
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
                Log::Memory<ELogLevel::Verbose>("Freeing from upstream at %p", BytePtr);
                Upstream->deallocate(SrcPtr, Bytes, Alignment);
                return nullptr;
            }

            DestroyBlock(AllocationHeader);

            Log::Memory<ELogLevel::Verbose>("Freeing %zu bytes at %p", AllocationHeader->Length, BytePtr);

            return nullptr;
        }

        /* Reallocating. */
        if (AllocationHeader->NextBlock != nullptr)
        {
            if (BytePtr + BytesWithHeader >= reinterpret_cast<std::byte*>(AllocationHeader->NextBlock) || !IsAlignedPtr(SrcPtr, Alignment))
            {
                Log::Memory<ELogLevel::Verbose>("Pending reallocation of %zu bytes into %zu at %p", AllocationHeader->Length, Bytes, BytePtr);
                /* Can't go beyond NextBlock! */
                ReallocBytes = std::min(AllocationHeader->Length, Bytes);
                DestroyBlock(AllocationHeader);
            }
            else
            {
                Log::Memory<ELogLevel::Verbose>("Reallocating %zu bytes at %p", Bytes, BytePtr);
                AllocationHeader->Length = Bytes;
                return SrcPtr;
            }
        }
        else
        {
            if (BytePtr + BytesWithHeader >= Buffer.data() + Buffer.size() || !IsAlignedPtr(SrcPtr, Alignment))
            {
                Log::Memory<ELogLevel::Verbose>("Pending reallocation of %zu bytes into %zu at %p", AllocationHeader->Length, Bytes, BytePtr);
                /* Can't go beyond end of the buffer! */
                ReallocBytes = std::min(AllocationHeader->Length, Bytes);
                DestroyBlock(AllocationHeader);
            }
            else
            {
                Log::Memory<ELogLevel::Verbose>("Reallocating %zu bytes at %p", Bytes, BytePtr);
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

        Log::Memory<ELogLevel::Verbose>("Allocating %zu bytes from upstream at %p", Bytes, NewPtr);

        if (ReallocBytes != 0)
        {
            Log::Memory<ELogLevel::Verbose>("Copying %zu bytes to %p", std::min(ReallocBytes, Bytes), NewPtr);
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

                            Log::Memory<ELogLevel::Verbose>("Allocating %zu bytes from upstream at %p", Bytes, NewPtr);

                            if (ReallocBytes != 0)
                            {
                                Log::Memory<ELogLevel::Verbose>("Copying %zu bytes to %p", std::min(ReallocBytes, Bytes), NewPtr);
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
        Log::Memory<ELogLevel::Verbose>("Failed to allocate %zu bytes", Bytes);
        exit(1);
    }
    else
    {
        /* Shift pointer to point to allocated data. */
        NewPtr = static_cast<std::byte*>(AlignPtr(NewPtr + sizeof(SAllocationHeader), Alignment));
#ifdef EQUINOX_REACH_DEVELOPMENT
        if (ReallocBytes == 0)
        {
            Log::Memory<ELogLevel::Verbose>("Allocating %zu bytes at %p", Bytes, NewPtr);
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
        Log::Memory<ELogLevel::Verbose>("Copying %zu bytes to %p", std::min(ReallocBytes, Bytes), NewPtr);
        std::memcpy(NewPtr, SrcPtr, std::min(ReallocBytes, Bytes));
    }

    return NewPtr;
}

void* CTopmostResource::do_allocate(size_t Bytes, size_t Align)
{
    Log::Memory<ELogLevel::Critical>("Allocating %d bytes through topmost resource", Bytes);
    return ::operator new(Bytes, std::align_val_t(Align));
}

void CTopmostResource::do_deallocate(void* Pointer, size_t Bytes, size_t Align) noexcept

{
    Log::Memory<ELogLevel::Critical>("Freeing %p through topmost resource", Pointer);
    ::operator delete(Pointer, std::align_val_t(Align));
}

CMemory CMemory::Instance{};

namespace Memory
{
    void* Malloc(size_t Bytes)
    {
        void* Ptr = CMemory::Instance.InlineResource.do_allocate(Bytes, alignof(std::max_align_t));
        return Ptr;
    }

    void* Calloc(size_t Num, size_t Bytes)
    {
        void* Ptr = CMemory::Instance.InlineResource.do_allocate(Num * Bytes, alignof(std::max_align_t));
        std::memset(Ptr, 0, Bytes * Num);
        return Ptr;
    }

    void* Realloc(void* Ptr, size_t Bytes)
    {
        return CMemory::Instance.InlineResource.do_reallocate(Ptr, Bytes);
    }

    void Free(void* Ptr)
    {
        CMemory::Instance.InlineResource.do_deallocate(Ptr, 0, alignof(std::max_align_t));
    }

    std::size_t NumberOfBlocks()
    {
        return CMemory::Instance.InlineResource.NumberOfBlocks();
    }
}