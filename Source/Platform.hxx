#pragma once

#include "CommonTypes.hxx"

struct SPlatform : SPlatformState
{
    struct SDL_Window* Window{};
    void* Context{};

    void Init();
    void Cleanup() const;

    [[nodiscard]] bool IsAnyFullscreen() const;

    void SwapBuffers() const;

    void ToggleBorderlessFullscreen() const;

    [[nodiscard]] static SVec2Int CalculateOptimalWindowedResolution(unsigned DisplayID);

    void SetOptimalWindowedResolution() const;
};
