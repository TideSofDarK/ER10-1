#include <memory>
#include "SDL3/SDL_main.h"
#include "Memory.hxx"
#include "Game.hxx"

int main([[maybe_unused]] int ArgCount, [[maybe_unused]] char* Arguments[])
{
    Memory::Init();

    auto Game = std::make_unique<SGame>();
    Game->Run();

    Memory::Shutdown();

    return 0;
}
