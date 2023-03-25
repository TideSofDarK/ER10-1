#include <SDL.h>

#include "Game.hxx"

int SDL_main(int argc, char *argv[]) {
    std::unique_ptr<SGame> Game = std::make_unique<SGame>();
    Game->Run();
    return 0;
}


