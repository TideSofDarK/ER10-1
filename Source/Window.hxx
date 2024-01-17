#pragma once

#include <SDL3/SDL_video.h>
#include "CommonTypes.hxx"

struct SWindow : SWindowData
{
    SDL_Window* Window{};
    SDL_GLContext Context{};

    SWindow();
    ~SWindow();

    [[nodiscard]] bool IsAnyFullscreen() const;

    void SwapBuffers() const;

    void OnWindowResized(int InWidth, int InHeight);

    void ToggleBorderlessFullscreen() const;
};
