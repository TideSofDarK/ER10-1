#pragma once

#include <vector>

struct SLevel {
    std::vector<unsigned int> Grid;
    unsigned int Width;
    unsigned int Height;

    SLevel(std::vector<unsigned int> &&InGrid, unsigned int InWidth, unsigned int InHeight);
};


