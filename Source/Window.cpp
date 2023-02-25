#include "Window.hpp"
#include "Constants.hpp"

#include <SDL.h>

#ifdef __WINDOWS__

#include <windows.h>
#include <dwmapi.h>
#include <VersionHelpers.h>
#include <iostream>

#endif

void SWindow::SwapBuffers() const {
#ifdef __WINDOWS__
    bool useDwmFlush = false;
    int swapInterval = SDL_GL_GetSwapInterval();

    // https://github.com/love2d/love/issues/1628
    // VSync can interact badly with Windows desktop composition (DWM) in windowed mode. DwmFlush can be used instead
    // of vsync, but it's much less flexible so we're very conservative here with where it's used:
    // - It won't work with exclusive or desktop fullscreen.
    // - DWM refreshes don't always match the refresh rate of the monitor the window is in (or the requested swap
    //   interval), so we only use it when they do match.
    // - The user may force GL vsync, and DwmFlush shouldn't be used together with GL vsync.
    if (!IsExclusiveFullscreen() && swapInterval == 1) {
        // Desktop composition is always enabled in Windows 8+. But DwmIsCompositionEnabled won't always return true...
        // (see DwmIsCompositionEnabled docs).
        BOOL compositionEnabled = IsWindows8OrGreater();
        if (compositionEnabled || (SUCCEEDED(DwmIsCompositionEnabled(&compositionEnabled)) && compositionEnabled)) {
            DWM_TIMING_INFO info = {};
            info.cbSize = sizeof(DWM_TIMING_INFO);
            double dwmRefreshRate = 0;
            if (SUCCEEDED(DwmGetCompositionTimingInfo(nullptr, &info)))
                dwmRefreshRate = (double) info.rateRefresh.uiNumerator / (double) info.rateRefresh.uiDenominator;

            SDL_DisplayMode dmode = {};
            int displayindex = SDL_GetWindowDisplayIndex(Window);

            if (displayindex >= 0)
                SDL_GetCurrentDisplayMode(displayindex, &dmode);

            if (dmode.refresh_rate > 0 && dwmRefreshRate > 0 && (fabs(dmode.refresh_rate - dwmRefreshRate) < 2)) {
                SDL_GL_SetSwapInterval(0);
                if (SDL_GL_GetSwapInterval() == 0)
                    useDwmFlush = true;
                else
                    SDL_GL_SetSwapInterval(swapInterval);
            }
        }
    }
#endif

    SDL_GL_SwapWindow(Window);

#ifdef __WINDOWS__
    if (useDwmFlush) {
        DwmFlush();
        SDL_GL_SetSwapInterval(swapInterval);
    }
#endif
}

bool SWindow::IsExclusiveFullscreen() const {
    return (SDL_GetWindowFlags(Window) & SDL_WINDOW_FULLSCREEN) != 0;
}

void SWindow::Init() {
    if (Window != nullptr) {
        std::cout << "Window is not nullptr!" << std::endl;
        quick_exit(1);
    }

    SDL_Init(SDL_INIT_EVERYTHING);

    Window = SDL_CreateWindow(GAME_NAME,
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              WINDOW_WIDTH, WINDOW_HEIGHT,
                              SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    Context = SDL_GL_CreateContext(Window);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetSwapInterval(1);

    if (!Window) {
        std::cout << "Window could not be created!" << std::endl
                  << "SDL_Error: " << SDL_GetError() << std::endl;
        quick_exit(1);
    }

    SDL_GetWindowSizeInPixels(Window, &Width, &Height);
    OnWindowResized(Width, Height);
}

void SWindow::OnWindowResized(int InWidth, int InHeight) {
    Width = InWidth;
    Height = InHeight;
    BlitWidth = Width;
    BlitHeight = static_cast<int>(static_cast<float>(BlitWidth) / SCREEN_ASPECT);
    if (Height < BlitHeight) {
        BlitHeight = Height;
        BlitWidth = static_cast<int>(static_cast<float>(BlitHeight) * SCREEN_ASPECT);
    }
    BlitX = max((Width - BlitWidth) / 2, 0);
    BlitY = max((Height - BlitHeight) / 2, 0);
}

void SWindow::Cleanup() const {
    SDL_GL_DeleteContext(Context);
    SDL_DestroyWindow(Window);
    SDL_Quit();
}

void SWindow::ToggleBorderlessFullscreen() const {
    const auto Flags = (SDL_GetWindowFlags(Window) & SDL_WINDOW_FULLSCREEN_DESKTOP) != 0 ? 0
                                                                                         : SDL_WINDOW_FULLSCREEN_DESKTOP;
    SDL_SetWindowFullscreen(Window, Flags);
}
