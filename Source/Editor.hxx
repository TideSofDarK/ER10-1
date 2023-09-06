#pragma once

#include <memory>
#include "Level.hxx"

struct SEditor {
    bool bLevelEditorActive;
    UVec2Int NewLevelSize;
    int LevelEditorCellSize;
    bool bDrawWallJoints;
    bool bDrawEdges;
    bool bDrawGridLines;
    std::optional<UVec2Int> SelectedTileCoords;
    std::shared_ptr<SLevel> Level;

    SEditor() {
        bLevelEditorActive = false;
        NewLevelSize = UVec2Int{8, 8};
        LevelEditorCellSize = 32;
        bDrawWallJoints = true;
        bDrawEdges = true;
        bDrawGridLines = false;
        Level = nullptr;
    }

    static void Init(struct SDL_Window *Window, void *Context);

    static void Cleanup();

    static void ProcessEvent(const union SDL_Event *Event);

    void Update();

    void Draw() const;

    [[nodiscard]] bool IsLevelLoaded() const {
        return Level != nullptr;
    }

private:
    void DrawLevel();
};