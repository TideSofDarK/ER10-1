#include "Level.hpp"

SLevel::SLevel(std::vector<unsigned int> &&InGrid, unsigned int InWidth, unsigned int InHeight) {
    Grid = InGrid;
    Width = InWidth;
    Height = InHeight;
}
