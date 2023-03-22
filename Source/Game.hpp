#pragma once

#include <memory>
#include "CommonTypes.hpp"
#include "Draw.hpp"
#include "Player.hpp"
#include "Window.hpp"
#include "Level.hpp"

struct SGame {
private:
    static EKeyState UpdateKeyState(EKeyState OldKeyState, const uint8_t *KeyboardState, uint8_t Scancode);

public:
    SWindow Window{};

    SRenderer Renderer;
    SInputState OldInputState{}, InputState{};
    SCamera Camera;
    SPlayer Player;
    SLevel Level;

    SSpriteHandle NoiseSprite;
    SSpriteHandle RefSprite;
    SSpriteHandle FrameSprite;
    SSpriteHandle AngelSprite;

    SGame();

    void Run();
};
