#include "Game.hpp"

#include <SDL_image.h>
#include "Level.hpp"
#include "Constants.hpp"

SGame::SGame() {
    Window.Init();

    Renderer.Init(SCREEN_WIDTH, SCREEN_HEIGHT);

    Player.X = 1;
    Player.Y = 1;
    Player.SetDirection(EDirection::West, true);

    Camera.Regenerate(45.0f, SCENE_ASPECT);

    SLevel Level{
            {
                    1, 1, 1, 1, 1, 1, 1, 1,
                    1, 0, 0, 0, 0, 0, 0, 1,
                    1, 0, 0, 0, 0, 0, 0, 1,
                    1, 0, 1, 0, 0, 1, 0, 1,
                    1, 0, 1, 0, 0, 1, 0, 1,
                    1, 0, 1, 1, 1, 1, 0, 1,
                    1, 0, 0, 0, 0, 0, 0, 1,
                    1, 1, 1, 1, 1, 1, 1, 1,
            },
            8,
            8
    };

    LevelGeometry.InitFromLevel(Level);

    auto FrameImage = IMG_Load(ASSETS_PATH"Frame.png");
    FrameTexture.InitFromPixels(FrameImage->w, FrameImage->h, FrameImage->pixels);
    SDL_FreeSurface(FrameImage);
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
        Renderer.Draw3D({0.0f, 0.0f, 0.0f}, &LevelGeometry);
        Renderer.Draw2D({0.0f, 0.0f, 0.0f}, {SCREEN_WIDTH, SCREEN_HEIGHT}, &FrameTexture);
        Renderer.DrawHUD({32.0f, 32.0f, 0.0f}, {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2});
//        Renderer.Draw2D(&FrameTexture, {32.0f, 32.0f, 0.0f},
//                        {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2});
//        Renderer.Draw2D(&FrameTexture, {64.0f, 64.0f, 0.0f}, {SCREEN_WIDTH / 3, SCREEN_HEIGHT / 3});
        Renderer.Flush(Window);

        Window.SwapBuffers();
    }

    FrameTexture.Cleanup();
    LevelGeometry.Cleanup();
    Renderer.Cleanup();
    Window.Cleanup();
}