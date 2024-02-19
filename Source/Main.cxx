#include <SDL3/SDL_main.h>
#include "Memory.hxx"
#include "Game.hxx"

void EquinoxReach()
{
    auto Game = Memory::MakeShared<SGame>();
    Game->Run();
}

#ifdef __cplusplus
extern "C"
#endif
    int
    main(int argc, char* argv[])
{
    EquinoxReach();

    return 0;
}
