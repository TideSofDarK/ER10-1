#include "Game.hxx"

#include <iostream>
#include "Constants.hxx"
#include "AssetTools.hxx"
#include "SDL.h"

namespace Asset::Common {
    EXTERN_ASSET(FramePNG)
    EXTERN_ASSET(RefPNG)
    EXTERN_ASSET(AngelPNG)
    EXTERN_ASSET(NoisePNG)
    EXTERN_ASSET(QuadOBJ)
    EXTERN_ASSET(PillarOBJ)
}

namespace Asset::TileSet::Hotel {
    EXTERN_ASSET(FloorOBJ)
    EXTERN_ASSET(WallOBJ)
    EXTERN_ASSET(WallJointOBJ)
    EXTERN_ASSET(DoorOBJ)
    EXTERN_ASSET(AtlasPNG)
}

SGame::SGame() {
    Window.Init();

#ifdef EQUINOX_REACH_DEVELOPMENT
    Editor.Init(Window.Window, Window.Context);
#endif

    Renderer.Init(SCREEN_WIDTH, SCREEN_HEIGHT);

    auto &CommonAtlas2D = Renderer.Atlases[ATLAS_COMMON];
    NoiseSprite = CommonAtlas2D.AddSprite(
            Asset::Common::NoisePNG);
    RefSprite = CommonAtlas2D.AddSprite(
            Asset::Common::RefPNG);
    CommonAtlas2D.Build();

    auto &PrimaryAtlas2D = Renderer.Atlases[ATLAS_PRIMARY2D];
    AngelSprite = PrimaryAtlas2D.AddSprite(
            Asset::Common::AngelPNG);
    FrameSprite = PrimaryAtlas2D.AddSprite(
            Asset::Common::FramePNG);
    PrimaryAtlas2D.Build();

    auto &PrimaryAtlas3D = Renderer.Atlases[ATLAS_PRIMARY3D];
    PrimaryAtlas3D.AddSprite(
            Asset::TileSet::Hotel::AtlasPNG);
    PrimaryAtlas3D.Build();

    {
        auto ScratchBuffer = Memory::GetScratchBuffer();
        auto FloorMesh = CRawMesh(
                Asset::TileSet::Hotel::FloorOBJ, ScratchBuffer);
        Floor.InitFromRawMesh(FloorMesh);
        TestGeometry.InitFromRawMesh(CRawMesh(
                Asset::Common::PillarOBJ, ScratchBuffer));
    }

    Renderer.TileSet.InitBasic(
            Asset::TileSet::Hotel::FloorOBJ, Asset::TileSet::Hotel::WallOBJ,
            Asset::TileSet::Hotel::WallJointOBJ, Asset::TileSet::Hotel::DoorOBJ);

    Player.ApplyDirection(true);

    Camera.RegenerateProjection();

    Level = SLevel{
            5,
            5,
            {
                    STile::WallNWS(), STile::WallN(), STile::WallN(), STile::WallNE(), STile::WallSW(false),
                    STile::WallNEW(), STile::WallW(), STile::WallS(), STile::Floor(), STile::WallNE(),
                    STile::WallWE(), STile::WallWE(), STile::WallsNF(), STile::WallW(), STile::WallE(),
                    STile::WallWE(), STile::WallSWE(), STile::WallNW(), STile::Floor(), STile::WallE(),
                    STile::WallSW(), STile::WallNS(), STile::WallS(), STile::WallS(), STile::WallSE(),
            }
    };
    Level.InitWallJoints();

#ifdef EQUINOX_REACH_DEVELOPMENT
    Editor.Level = std::make_shared<SLevel>(Level);
#endif

    SpriteDemoState = 5;
}

EKeyState SGame::UpdateKeyState(EKeyState OldKeyState, const uint8_t *KeyboardState, const uint8_t Scancode) {
    bool bCurrentlyPressed = KeyboardState[Scancode] == 1;
    bool bWasPressed = OldKeyState == EKeyState::Down || OldKeyState == EKeyState::Pressed;
    if (bCurrentlyPressed) {
        if (bWasPressed) {
            return EKeyState::Down;
        } else {
            return EKeyState::Pressed;
        }
    } else {
        if (bWasPressed) {
            return EKeyState::Released;
        } else {
            return EKeyState::None;
        }
    }
}

void SGame::Run() {
    Window.Now = SDL_GetTicks64();
    while (!Window.bQuit) {
        Window.Last = Window.Now;
        Window.Now = SDL_GetTicks64();
        Window.DeltaTime = static_cast<float>( Window.Now - Window.Last) / 1000.0f;
        Window.Seconds += Window.DeltaTime;

        SDL_Event Event;
        while (SDL_PollEvent(&Event)) {
#ifdef EQUINOX_REACH_DEVELOPMENT
            Editor.ProcessEvent(&Event);
#endif
            switch (Event.type) {
                case SDL_WINDOWEVENT:
                    if (Event.window.event == SDL_WINDOWEVENT_RESIZED) {
                        Window.OnWindowResized(Event.window.data1, Event.window.data2);
                    }
                    break;
                case SDL_QUIT:
                    Window.bQuit = true;
                    break;
                default:
                    break;
            }
        }

#ifdef EQUINOX_REACH_DEVELOPMENT
        Editor.Update();
#endif

#pragma region InputHandling

        OldInputState = InputState;
        const Uint8 *KeyboardState = SDL_GetKeyboardState(nullptr);
        InputState.Up = UpdateKeyState(OldInputState.Up, KeyboardState, SDL_SCANCODE_W);
        InputState.Right = UpdateKeyState(OldInputState.Right, KeyboardState, SDL_SCANCODE_D);
        InputState.Down = UpdateKeyState(OldInputState.Down, KeyboardState, SDL_SCANCODE_S);
        InputState.Left = UpdateKeyState(OldInputState.Left, KeyboardState, SDL_SCANCODE_A);
        InputState.L = UpdateKeyState(OldInputState.L, KeyboardState, SDL_SCANCODE_Q);
        InputState.R = UpdateKeyState(OldInputState.R, KeyboardState, SDL_SCANCODE_E);
        InputState.ZL = UpdateKeyState(OldInputState.ZL, KeyboardState, SDL_SCANCODE_Z);
        InputState.ZR = UpdateKeyState(OldInputState.ZR, KeyboardState, SDL_SCANCODE_C);
        InputState.Accept = UpdateKeyState(OldInputState.Accept, KeyboardState, SDL_SCANCODE_SPACE);
        InputState.Cancel = UpdateKeyState(OldInputState.Cancel, KeyboardState, SDL_SCANCODE_ESCAPE);
        InputState.ToggleFullscreen = UpdateKeyState(OldInputState.ToggleFullscreen, KeyboardState, SDL_SCANCODE_F11);
        InputState.ToggleLevelEditor = UpdateKeyState(OldInputState.ToggleLevelEditor, KeyboardState, SDL_SCANCODE_F10);
#pragma endregion

        if (InputState.Cancel == EKeyState::Pressed) {
            Window.bQuit = true;
        }

        if (InputState.ToggleFullscreen == EKeyState::Pressed) {
            Window.ToggleBorderlessFullscreen();
        }

#ifdef EQUINOX_REACH_DEVELOPMENT
        if (InputState.ToggleLevelEditor == EKeyState::Pressed) {
            Editor.bLevelEditorActive = !Editor.bLevelEditorActive;
        }
#endif

#pragma region GameLoop
        if (IsGameRunning()) {
#ifdef EQUINOX_REACH_DEVELOPMENT
            bool bImportLevel{};
            Editor.DebugTools(&bImportLevel);

            if (bImportLevel) {
                Level = *Editor.Level;
                Level.InitWallJoints();
            }
#endif

            if (!Player.IsMoving()) {
                if (InputState.Up == EKeyState::Down) {
                    if (CheckIfPlayerCanMove()) {
                        Player.MoveForward();
                    } else {
                        Player.BumpIntoWall();
                    }
                }
                if (InputState.Left == EKeyState::Down) {
                    Player.Turn(false);
                } else if (InputState.Right == EKeyState::Down) {
                    Player.Turn(true);
                }
            }

            if (InputState.L == EKeyState::Pressed) {
                SpriteDemoState = std::max(0, SpriteDemoState - 1);
            }

            if (InputState.R == EKeyState::Pressed) {
                SpriteDemoState = std::min(5, SpriteDemoState + 1);
            }

            Player.Update(Window.DeltaTime);

            Camera.Position = Player.EyePositionCurrent;
            Camera.Target = Camera.Position + Player.EyeForwardCurrent;
            Camera.Update();

            Renderer.UploadProjectionAndViewFromCamera(Camera);
//        Renderer.Draw3D({0.0f, 0.0f, 0.0f}, &LevelGeometry);
//        Renderer.Draw3D({4.0f, 0.0f, -2.0f}, &Renderer.Tileset);
            Renderer.Draw3D({-3.0f, 0.0f, -3.0f}, &TestGeometry);
            Renderer.Draw3D({-4.0f, 0.0f, -4.0f}, &Floor);
            Renderer.Draw3D({-5.0f, 0.0f, -4.0f}, &Floor);
            Renderer.Draw3D({-6.0f, 0.0f, -4.0f}, &Floor);
            Renderer.Draw3D({-7.0f, 0.0f, -4.0f}, &Floor);
            Renderer.Draw3DLevel(Level, Player.Coords, Player.Direction);
            switch (SpriteDemoState) {
                case 0:
                    Renderer.Draw2DEx({220, 80.0f, 0.0f}, AngelSprite, UBER2D_MODE_DISINTEGRATE_PLASMA,
                                      {Window.Seconds / 2.0f, 0.9f, 0.2f, 0.1f},
                                      NoiseSprite.Sprite->UVRect);
                    break;
                case 1:
                    Renderer.Draw2DHaze({220, 80.0f, 0.0f}, AngelSprite, 0.07f, 4.0f, 4.0f);
                    break;
                case 2:
                    Renderer.Draw2DBackBlur({220, 80.0f, 0.0f}, AngelSprite, 4.0f, 2.9f, 0.08f);
                    break;
                case 3:
                    Renderer.Draw2DGlow({220, 80.0f, 0.0f}, AngelSprite, {1.0f, 1.0f, 1.0f}, 2.0f);
                    break;
                case 4:
                    Renderer.Draw2DDisintegrate({220, 80.0f, 0.0f}, AngelSprite, NoiseSprite, Window.Seconds / 4.0f);
                    break;
                default:
                    break;
            }

//            Renderer.DrawHUD({32.0f, 250.0f, 0.0f}, {SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4}, HUD_MODE_BORDER_DASHED);
//            Renderer.DrawHUD({128.0f, 250.0f, 0.0f}, {SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4}, HUD_MODE_BUTTON);

            Renderer.Flush(Window);
        }
#pragma endregion

#ifdef EQUINOX_REACH_DEVELOPMENT
        Editor.Draw();
#endif

        Window.SwapBuffers();
    }

#ifdef EQUINOX_REACH_DEVELOPMENT
    Editor.Cleanup();
#endif
    Renderer.Cleanup();
    Window.Cleanup();
}

bool SGame::CheckIfPlayerCanMove() const {
    auto DirectionVector = Player.Direction.DirectionVectorFromDirection<int>();

    auto CurrentTile = Level.GetTileAt(Player.Coords);
    if (CurrentTile == nullptr) {
        return false;
    }

    if (!CurrentTile->IsTraversable(Player.Direction.Index)) {
        return false;
    }

    auto NextTile = Level.GetTileAt(Player.Coords + DirectionVector);
    if (NextTile == nullptr || !NextTile->IsWalkable()) {
        return false;
    }

    return true;
}

bool SGame::IsGameRunning() const {
#ifdef EQUINOX_REACH_DEVELOPMENT
    return !Editor.bLevelEditorActive;
#endif
    return true;
}
