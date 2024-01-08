#pragma once

#include "Math.hxx"

struct SWindowData
{
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

struct SDirection
{
    using Type = unsigned;
    static constexpr Type Count = 4;
    Type Index : 2;

    void CycleCW()
    {
        Index++;
    }

    void CycleCCW()
    {
        Index--;
    }

    constexpr static const char* Names[] = { "North", "East", "South", "West" };

    constexpr static SDirection North()
    {
        return SDirection{ 0 };
    }

    constexpr static SDirection East()
    {
        return SDirection{ 1 };
    }

    constexpr static SDirection South()
    {
        return SDirection{ 2 };
    }

    constexpr static SDirection West()
    {
        return SDirection{ 3 };
    }

    template <typename T>
    constexpr SVec2<T> GetVector() const
    {
        switch (Index)
        {
            case North().Index:
                return { 0, -1 };
            case East().Index:
                return { 1, 0 };
            case South().Index:
                return { 0, 1 };
            case West().Index:
                return { -1, 0 };
            default:
                return {};
        }
    }

    [[nodiscard]] constexpr float RotationFromDirection() const
    {
        switch (Index)
        {
            case East().Index:
                return Math::PI * -0.5f;
            case South().Index:
                return Math::PI;
            case West().Index:
                return Math::PI / 2.0f;
            default:
                return 0.0f;
        }
    }
};

enum class EKeyState : unsigned
{
    None,
    Pressed,
    Held,
    Released
};

struct SInputState
{
    EKeyState Up : 2;
    EKeyState Right : 2;
    EKeyState Down : 2;
    EKeyState Left : 2;
    EKeyState Accept : 2;
    EKeyState Cancel : 2;
    EKeyState L : 2;
    EKeyState ZL : 2;
    EKeyState R : 2;
    EKeyState ZR : 2;

    EKeyState ToggleFullscreen : 2;

#ifdef EQUINOX_REACH_DEVELOPMENT
    EKeyState ToggleLevelEditor : 2;
#endif
};
