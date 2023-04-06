#pragma once

#include "CommonTypes.hxx"
#include "Draw.hxx"
#include "Player.hxx"
#include "Window.hxx"
#include "Level.hxx"

#ifdef EQUINOX_REACH_DEVELOPMENT

#include "Editor.hxx"

#endif

struct SGame {
private:
    static EKeyState UpdateKeyState(EKeyState OldKeyState, const uint8_t *KeyboardState, uint8_t Scancode);

public:
    SWindow Window{};

#ifdef EQUINOX_REACH_DEVELOPMENT
    SEditor Editor;
#endif

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

    [[nodiscard]] bool CheckIfPlayerCanMove() const;
};
