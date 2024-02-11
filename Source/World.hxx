#pragma once

#include <array>
#include "Level.hxx"

inline constexpr int WorldMaxLevels = 8;

struct SWorldLevel
{
    SLevel Level;
};

struct SWorld
{
    std::array<SWorldLevel, WorldMaxLevels> Levels;
};
