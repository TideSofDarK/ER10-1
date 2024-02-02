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

    void OnWindowResized();

    void ToggleBorderlessFullscreen() const;

    [[nodiscard]] static UVec2Int CalculateOptimalWindowedResolution(unsigned DisplayID);

    void SetOptimalWindowedResolution() const;
};

namespace Platform
{
    void Init();
}
