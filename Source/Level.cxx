#include "Level.hxx"

STile const &SLevel::GetTileAt(int X, int Y) {
    auto Index = CoordsToIndex(X, Y);
    return Grid[Index];
}
