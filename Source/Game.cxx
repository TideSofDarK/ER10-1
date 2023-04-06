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
    EXTERN_ASSET(AtlasPNG)
}

SGame::SGame() {
    Window.Init();

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
            Asset::TileSet::Hotel::WallJointOBJ);

    Player.ApplyDirection(true);

    Camera.RegenerateProjection();

    Level = SLevel{
            5,
            5,
            {
                    STile::WallNWS(), STile::WallN(), STile::WallN(), STile::WallNE(), STile::WallSW(),
                    STile::WallNEW(), STile::WallW(), STile::WallS(), STile(), STile::WallNE(),
                    STile::WallWE(), STile::WallWE(), STile::WallsNF(), STile::WallW(), STile::WallE(),
                    STile::WallWE(), STile::WallSWE(), STile::WallNW(), STile(), STile::WallE(),
                    STile::WallSW(), STile::WallNS(), STile::WallS(), STile::WallS(), STile::WallSE(),
            }
    };
    Level.InitWallJoints();
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

        /** Input processing */
        OldInputState = InputState;
        const Uint8 *KeyboardState = SDL_GetKeyboardState(nullptr);
        InputState.Up = UpdateKeyState(OldInputState.Up, KeyboardState, SDL_SCANCODE_W);
        InputState.Right = UpdateKeyState(OldInputState.Right, KeyboardState, SDL_SCANCODE_D);
        InputState.Down = UpdateKeyState(OldInputState.Down, KeyboardState, SDL_SCANCODE_S);
        InputState.Left = UpdateKeyState(OldInputState.Left, KeyboardState, SDL_SCANCODE_A);
        InputState.L = UpdateKeyState(OldInputState.Left, KeyboardState, SDL_SCANCODE_Q);
        InputState.R = UpdateKeyState(OldInputState.Left, KeyboardState, SDL_SCANCODE_E);
        InputState.ZL = UpdateKeyState(OldInputState.Left, KeyboardState, SDL_SCANCODE_Z);
        InputState.ZR = UpdateKeyState(OldInputState.Left, KeyboardState, SDL_SCANCODE_C);
        InputState.Accept = UpdateKeyState(OldInputState.Accept, KeyboardState, SDL_SCANCODE_SPACE);
        InputState.Cancel = UpdateKeyState(OldInputState.Cancel, KeyboardState, SDL_SCANCODE_ESCAPE);
        InputState.ToggleFullscreen = UpdateKeyState(OldInputState.ToggleFullscreen, KeyboardState, SDL_SCANCODE_F11);

#pragma region InputHandling
        if (InputState.Cancel == EKeyState::Pressed) {
            Window.bQuit = true;
        }

        if (InputState.ToggleFullscreen == EKeyState::Pressed) {
            Window.ToggleBorderlessFullscreen();
        }

        if (!Player.IsMoving()) {
            if (InputState.Up == EKeyState::Down) {
                if (CheckIfPlayerCanMove()) {
                    Player.MoveForward();
                }
            }
            if (InputState.Left == EKeyState::Down) {
                Player.Turn(false);
            } else if (InputState.Right == EKeyState::Down) {
                Player.Turn(true);
            }
        }
#pragma endregion

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
        Renderer.Draw2DEx({30.0f - 4, 50.0f, 0.0f}, AngelSprite, UBER2D_MODE_DISINTEGRATE_PLASMA,
                          {Window.Seconds / 2.0f, 0.9f, 0.2f, 0.1f},
                          NoiseSprite.Sprite->UVRect);
//        Renderer.Draw2DHaze({85 - 4, 50.0f, 0.0f}, AngelSprite, 0.07f, 4.0f, 4.0f);
//        Renderer.Draw2DBackBlur({140 - 4, 50.0f, 0.0f}, AngelSprite, 4.0f, 2.9f, 0.08f);
//        Renderer.Draw2DGlow({195 - 4, 50.0f, 0.0f}, AngelSprite, glm::vec3(1.0f, 1.0f, 1.0f), 2.0f);
//        Renderer.Draw2DDisintegrate({250 - 4, 50.0f, 0.0f}, AngelSprite, NoiseSprite, Window.Seconds / 4.0f);
//        Renderer.Draw2D({0.0f, 0.0f, 0.0f}, FrameSprite);
        Renderer.DrawHUD({32.0f, 250.0f, 0.0f}, {SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4}, HUD_MODE_BORDER_DASHED);
        Renderer.DrawHUD({128.0f, 250.0f, 0.0f}, {SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4}, HUD_MODE_BUTTON);
        Renderer.Flush(Window);

        Window.SwapBuffers();
    }

    std::printf("EyeHeight: %f\n", Player.EyeHeight);
    std::printf("FieldOfViewY: %f\n", Camera.FieldOfViewY);
    std::printf("Aspect: %f\n", Camera.Aspect);

    Renderer.Cleanup();
    Window.Cleanup();
}

bool SGame::CheckIfPlayerCanMove() {
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