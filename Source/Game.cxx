#include "Game.hxx"

#include <iostream>
#include "Constants.hxx"
#include "Asset.hxx"
#include "SDL.h"

DEFINE_RESOURCE(Frame_png)
DEFINE_RESOURCE(Ref_png)
DEFINE_RESOURCE(Angel_png)
DEFINE_RESOURCE(Noise_png)
DEFINE_RESOURCE(Quad_obj)
DEFINE_RESOURCE(Pillar_obj)
DEFINE_RESOURCE(HotelFloor_obj)
DEFINE_RESOURCE(HotelWall_obj)
DEFINE_RESOURCE(HotelWallJoint_obj)
DEFINE_RESOURCE(HotelAtlas_png)

SGame::SGame() {
    Window.Init();

    Renderer.Init(SCREEN_WIDTH, SCREEN_HEIGHT);

    {
        auto ScratchBuffer = Memory::GetScratchBuffer();

        auto &CommonAtlas2D = Renderer.Atlases[ATLAS_COMMON];
        NoiseSprite = CommonAtlas2D.AddSprite(ResourceNoise_png, ScratchBuffer);
        RefSprite = CommonAtlas2D.AddSprite(ResourceRef_png, ScratchBuffer);
        CommonAtlas2D.Build(ScratchBuffer);

        auto &PrimaryAtlas2D = Renderer.Atlases[ATLAS_PRIMARY2D];
        AngelSprite = PrimaryAtlas2D.AddSprite(ResourceAngel_png, ScratchBuffer);
        FrameSprite = PrimaryAtlas2D.AddSprite(ResourceFrame_png, ScratchBuffer);
        PrimaryAtlas2D.Build(ScratchBuffer);

        auto &PrimaryAtlas3D = Renderer.Atlases[ATLAS_PRIMARY3D];
        PrimaryAtlas3D.AddSprite(ResourceHotelAtlas_png, ScratchBuffer);
        PrimaryAtlas3D.Build(ScratchBuffer);

        auto FloorMesh = CRawMesh(ResourceHotelFloor_obj, ScratchBuffer);
        Floor.InitFromRawMesh(FloorMesh);
        TestGeometry.InitFromRawMesh(CRawMesh(ResourcePillar_obj, ScratchBuffer));

        Renderer.Tileset.InitBasic(ResourceHotelFloor_obj, ResourceHotelWall_obj, ResourceHotelWallJoint_obj, ScratchBuffer);
    }

    Player.X = 0;
    Player.Y = 0;
    Player.SetDirection(EDirection::North, true);

    Camera.RegenerateProjection();

    Level = SLevel{
            5,
            5,
            {
                    STile::WallNorthWest(), STile::WallNorth(), STile::WallNorth(), STile::WallNorth(),
                    STile::WallNorthEast(),
                    STile::WallWestEast(), STile(), STile(), STile(), STile::WallEast(),
                    STile::WallWestEast(), STile(), STile::WallsNoFloor(), STile(), STile::WallEast(),
                    STile::WallWestEast(), STile(), STile(), STile(), STile::WallEast(),
                    STile::WallSouthWest(), STile::WallSouth(), STile::WallSouth(), STile::WallSouth(),
                    STile::WallSouthEast(),
            }
    };
}

EKeyState SGame::UpdateKeyState(EKeyState OldKeyState, const uint8_t *KeyboardState, const uint8_t Scancode) {
    bool bCurrentlyPressed = KeyboardState[Scancode] == 1;
    bool bWasPressed = OldKeyState == EKeyState::Press || OldKeyState == EKeyState::Pressed;
    if (bCurrentlyPressed) {
        if (bWasPressed) {
            return EKeyState::Press;
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

        if (InputState.Cancel == EKeyState::Pressed) {
            Window.bQuit = true;
        }

        /** Toggle borderless fullscreen */
        if (InputState.ToggleFullscreen == EKeyState::Pressed) {
            Window.ToggleBorderlessFullscreen();
        }

        Player.HandleInput(InputState);
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
        Renderer.Draw3DLevel(Level);
        Renderer.Draw2DEx({30.0f - 4, 50.0f, 0.0f}, AngelSprite, UBER2D_MODE_DISINTEGRATE_PLASMA,
                          {Window.Seconds / 2.0, 0.9f, 0.2f, 0.1f},
                          NoiseSprite.Sprite->UVRect);
//        Renderer.Draw2DHaze({85 - 4, 50.0f, 0.0f}, AngelSprite, 0.07f, 4.0f, 4.0f);
//        Renderer.Draw2DBackBlur({140 - 4, 50.0f, 0.0f}, AngelSprite, 4.0f, 2.9f, 0.08f);
//        Renderer.Draw2DGlow({195 - 4, 50.0f, 0.0f}, AngelSprite, glm::vec3(1.0f, 1.0f, 1.0f), 2.0f);
//        Renderer.Draw2DDisintegrate({250 - 4, 50.0f, 0.0f}, AngelSprite, NoiseSprite, Window.Seconds / 4.0f);
        Renderer.Draw2D({0.0f, 0.0f, 0.0f}, FrameSprite);
        Renderer.DrawHUD({32.0f, 150.0f, 0.0f}, {SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4}, HUD_MODE_BORDER_DASHED);
        Renderer.DrawHUD({128.0f, 150.0f, 0.0f}, {SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4}, HUD_MODE_BUTTON);
        Renderer.Flush(Window);

        Window.SwapBuffers();
    }

    std::printf("EyeHeight: %f\n", Player.EyeHeight);
    std::printf("FieldOfViewY: %f\n", Camera.FieldOfViewY);
    std::printf("Aspect: %f\n", Camera.Aspect);

    Renderer.Cleanup();
    Window.Cleanup();
}