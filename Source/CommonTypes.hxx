#pragma once

#include <array>
#include <cstdint>
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
    float TimeScale = 1.0f;
    float Seconds{};
    float DeltaTime{};
    bool bQuit{};
};

struct SDirection
{
    using Type = uint32_t;
    static constexpr Type Count = 4;
    Type Index;

    [[nodiscard]] Type Value() const { return Index; }

    void RotateCW(Type Turns)
    {
        Index = (Index + Turns) & ~0x4;
    }

    void RotateCCW(Type Turns)
    {
        Index = (Index - Turns) & 0x3;
    }

    void CycleCW()
    {
        RotateCW(1);
    }

    void CycleCCW()
    {
        RotateCCW(1);
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

    constexpr static SDirection Forward()
    {
        return SDirection{ 0 };
    }

    constexpr static SDirection Right()
    {
        return SDirection{ 1 };
    }

    constexpr static SDirection Backward()
    {
        return SDirection{ 2 };
    }

    constexpr static SDirection Left()
    {
        return SDirection{ 3 };
    }

    static const std::array<SDirection, 4>& All()
    {
        static const std::array<SDirection, Count> Directions = { SDirection::North(), SDirection::East(), SDirection::South(), SDirection::West() };
        return Directions;
    }

    [[nodiscard]] inline SDirection Side() const
    {
        SDirection NewDirection{ Index };
        NewDirection.CycleCW();
        return NewDirection;
    }

    [[nodiscard]] inline SDirection Inverted() const
    {
        SDirection NewDirection{ Index };
        NewDirection.RotateCCW(2);
        return NewDirection;
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

    bool operator==(const SDirection& Other) const
    {
        return Index == Other.Index;
    }
};

struct SCoordsAndDirection
{
    UVec2 Coords;
    SDirection Direction{};
};

enum class EKeyState : unsigned
{
    None,
    Pressed,
    Released,
    Held,
};

struct SKeys
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

struct SInputState
{
    union
    {
        unsigned long Value;
        SKeys Keys;
    };

    void Buffer(const SInputState& Other)
    {
        this->Value |= Other.Value;
    }
};

struct STimeline
{
    float Speed = 1.0f;
    float Value = 1.0f;

    void Advance(float DeltaTime)
    {
        Value += DeltaTime * Speed;
        if (Value > 1.0f)
        {
            Value = 1.0f;
        }
    }

    void Reset()
    {
        Value = 0.0f;
    }

    void Finish()
    {
        Value = 1.0f;
    }

    [[nodiscard]] inline bool IsFinishedPlaying() const { return Value >= 1.0f; }
    [[nodiscard]] inline bool IsPlaying() const { return Value >= 0.0f && Value < 1.0f; }
};
