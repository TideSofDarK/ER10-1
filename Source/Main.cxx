#include "Memory.hxx"
#include <SDL.h>
#include "Game.hxx"

int SDL_main([[maybe_unused]] int ArgCount, [[maybe_unused]] char *Arguments[]) {
    Memory::Init();

    auto Game = std::make_unique<SGame>();
    Game->Run();

    Memory::Shutdown();

    return 0;
}


