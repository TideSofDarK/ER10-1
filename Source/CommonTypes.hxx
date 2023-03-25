#pragma once

struct SWindowData {
    int Width{};
    int Height{};
    int BlitWidth{};
    int BlitHeight{};
    int BlitX{};
    int BlitY{};
    unsigned long long Last{};
    unsigned long long Now{};
    float Seconds{};
    float DeltaTime{};
    bool bQuit{};
};

enum class EDirection : unsigned {
    North = 0,
    East,
    South,
    West,
    Count
};

#define DIRECTION_COUNT static_cast<unsigned>(EDirection::Count)

enum class EKeyState : unsigned {
    None,
    Pressed,
    Press,
    Released
};

struct SInputState {
    EKeyState Up: 2;
    EKeyState Right: 2;
    EKeyState Down: 2;
    EKeyState Left: 2;
    EKeyState Accept: 2;
    EKeyState Cancel: 2;
    EKeyState L: 2;
    EKeyState ZL: 2;
    EKeyState R: 2;
    EKeyState ZR: 2;

    EKeyState ToggleFullscreen: 2;
};
