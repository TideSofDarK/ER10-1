#pragma once

#include <memory>
#include "CommonTypes.hxx"
#include "Draw.hxx"
#include "Player.hxx"
#include "Window.hxx"
#include "Level.hxx"

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
    SGeometry TestGeometry;
    SGeometry Floor;

    SSpriteHandle NoiseSprite;
    SSpriteHandle RefSprite;
    SSpriteHandle FrameSprite;
    SSpriteHandle AngelSprite;

    SGame();

    void Run();
};
