#pragma once

#include "CommonTypes.hxx"

struct SWindow : SWindowData
{
    struct SDL_Window* Window{};
    void* Context{};

    SWindow();
    ~SWindow();

    [[nodiscard]] bool IsAnyFullscreen() const;

    void SwapBuffers() const;

    void OnWindowResized(int InWidth, int InHeight);

    void ToggleBorderlessFullscreen() const;

    void SetOptimalWindowedResolution() const;
};

namespace Platform
{
    void Init();
}