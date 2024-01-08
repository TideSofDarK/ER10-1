#pragma once

#include "CommonTypes.hxx"
#include "Draw.hxx"
#include "Blob.hxx"
#include "Battle.hxx"
#include "Window.hxx"
#include "Level.hxx"
#include "GameSystem.hxx"

#ifdef EQUINOX_REACH_DEVELOPMENT

#include "DevTools.hxx"

#endif

struct SGame {
private:
    static EKeyState UpdateKeyState(EKeyState OldKeyState, const uint8_t *KeyboardState, uint8_t Scancode);

    int SpriteDemoState;

public:
    SWindow Window{};

#ifdef EQUINOX_REACH_DEVELOPMENT
    SDevTools DevTools;
#endif

    SRenderer Renderer;
    SInputState OldInputState{}, InputState{};
    SCamera Camera;
    SParty PlayerParty;
    SBlob Blob;
    SBattle Battle;
    SLevel Level;
    SGeometry TestGeometry;
    SGeometry Floor;

    SSpriteHandle NoiseSprite;
    SSpriteHandle RefSprite;
    SSpriteHandle FrameSprite;
    SSpriteHandle AngelSprite;

    SGame();

    void Run();

    bool AttemptPlayerStep(SDirection Direction);

    [[nodiscard]] bool IsGameRunning() const;
};
