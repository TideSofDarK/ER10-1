#include "Memory.hxx"

CMemory& CMemory::Get()
{
    static CMemory MemoryInstance{};
    return MemoryInstance;
}
