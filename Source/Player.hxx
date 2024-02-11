#pragma once

#include "SharedConstants.hxx"

namespace EPlayerUpgrades
{
    enum : UFlagType
    {
        RevealShapeBlock = 1 << 0
    };
}

struct SPlayer
{
    UFlagType Upgrades{};

    [[nodiscard]] int ExploreRadius() const { return 3; }
};
