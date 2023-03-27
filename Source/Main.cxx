#include "Memory.hxx"
#include <SDL.h>
#include "Game.hxx"

int SDL_main([[maybe_unused]] int ArgCount, [[maybe_unused]] char *Arguments[]) {
    Memory::Init();

//    for (int i = 0; i < 8; ++i) {
//        {
//            auto ScratchBuffer = Memory::GetScratchBuffer<std::byte>();
//            auto v = ScratchBuffer.GetVector<std::byte>();
//            for (int j = 0; j < 512; ++j) {
//                v.emplace_back(std::byte(1));
//            }
//            auto v1 = ScratchBuffer.GetVector<int>();
//            for (int j = 0; j < 512; ++j) {
//                v1.emplace_back(32);
//            }
//            auto v2 = ScratchBuffer.GetVector<long>();
//            auto v3 = ScratchBuffer.GetVector<float>();
//            auto v4 = ScratchBuffer.GetVector<double>();
//        }
//    }

    auto Game = std::make_unique<SGame>();
    Game->Run();

    Memory::Shutdown();

    return 0;
}


