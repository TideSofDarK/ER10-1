#pragma once

#include "CommonTypes.hxx"
#include "Draw.hxx"
#include "Blob.hxx"
#include "Battle.hxx"
#include "Audio.hxx"
#include "Window.hxx"
#include "Level.hxx"
#include "GameSystem.hxx"
#include "Player.hxx"
#include "World.hxx"

#ifdef EQUINOX_REACH_DEVELOPMENT
    #include "DevTools.hxx"
#endif

struct SGame
{
private:
    static EKeyState UpdateKeyState(EKeyState OldKeyState, const uint8_t* KeyboardState, uint8_t Scancode);

    int SpriteDemoState;

public:
    SWindow Window;

#ifdef EQUINOX_REACH_DEVELOPMENT
    SDevTools DevTools;
#endif

    CAudio Audio;
    SRenderer Renderer;
    SInputState OldInputState{}, BufferedInputState{}, InputState{};
    SCamera Camera;
    SWorld World;
    SPlayer Player;
    SParty PlayerParty;
    SBlob Blob;
    SBattle Battle;
    SLevel Level;
    // SGeometry TestGeometry;
    STileset Tileset;

    STimeline MapRectTimeline{3.0f};
    URect MapRect;
    bool bMapMaximized{};

    SSoundClip DoorCreek{};

    std::array<SSpriteHandle, MAP_ICON_COUNT> MapIcons;
    SSpriteHandle NoiseSprite;
    SSpriteHandle RefSprite;
    SSpriteHandle FrameSprite;
    SSpriteHandle AngelSprite;

    SGame();

    void Run();

    void UpdateInputState();
    void HandleBlobMovement();
    bool AttemptBlobStep(SDirection Direction);
    void OnBlobMoved();
    void ChangeLevel();
    void ChangeLevel(const SLevel& NewLevel);
    void ChangeLevel(const SAsset& LevelAsset);

    [[nodiscard]] bool IsGameRunning() const;
};
