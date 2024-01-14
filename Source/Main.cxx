#include "Log.hxx"
#include "SDL3/SDL_main.h"
#include "Memory.hxx"
#include "Game.hxx"

int main([[maybe_unused]] int ArgCount, [[maybe_unused]] char* Arguments[])
{
    //    void* OldPtr{};
    //    for (auto i = 0; i < 31; i++) {
    //        auto Ptr = CMemory::Malloc(i * 16 + 1);
    //        if (i % 2 == 0) {
    //            CMemory::Free(OldPtr);
    //            OldPtr = Ptr;
    //        }
    //    }
    //    Log::Memory("-------------");
    //
    //    auto Ptr1 = CMemory::Malloc(8);
    //    auto Ptr2 = CMemory::Malloc(144);
    //    CMemory::Free(Ptr1);
    //    CMemory::Free(Ptr2);
    //    auto Ptr3 = CMemory::Malloc(56);
    // auto Ptr4 = CMemory::Malloc(256);
    // Ptr2 = CMemory::Realloc(Ptr2, 512);
    // CMemory::Free(Ptr2);
    // CMemory::Malloc(8);
    // CMemory::Malloc(8);
    // Log::Memory("Ptr1: %p Size: &zu", Ptr1, 32);

    auto Game = CMemory::MakeShared<SGame>();
    Game->Run();
    return 0;
}
