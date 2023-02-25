#pragma once

#include <memory>
#include "CommonTypes.hpp"
#include "Draw.hpp"
#include "Player.hpp"
#include "Window.hpp"

struct SGame {
private:
    static EKeyState UpdateKeyState(EKeyState OldKeyState, const uint8_t *KeyboardState, uint8_t Scancode);

public:
    SWindow Window{};

    SRenderer Renderer;
    SInputState OldInputState{}, InputState{};
    SCamera Camera;
    SPlayer Player;
    SLevelGeometry LevelGeometry;

    STexture FrameTexture{};

    SGame();

    void Run();

};
