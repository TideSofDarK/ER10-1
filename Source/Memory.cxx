#include "Memory.hxx"

CMemory MemoryInstance{};

CMemory& CMemory::Get()
{
    return MemoryInstance;
}