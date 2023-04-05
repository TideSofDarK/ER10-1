#include "Memory.hxx"
#include "SDL_main.h"
#include "Game.hxx"

int main([[maybe_unused]] int ArgCount, [[maybe_unused]] char *Arguments[]) {
    Memory::Init();

    auto Game = std::make_unique<SGame>();
    Game->Run();

    Memory::Shutdown();

    return 0;
}


