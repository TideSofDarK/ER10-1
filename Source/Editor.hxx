#pragma once

#include <memory>
#include "imgui.h"
#include "Level.hxx"

enum class ELevelEditorMode {
    Normal,
    ToggleDoor
};

struct SDebugToolsData {
    float FPS{};
    UVec2Int PlayerCoords{};
    SDirection PlayerDirection{0};
    bool bImportLevel{};
};

struct SEditor {
    bool bLevelEditorActive{};
    ELevelEditorMode LevelEditorMode{};
    UVec2Int NewLevelSize{};
    float LevelEditorCellSize{};
    bool bDrawWallJoints{};
    bool bDrawEdges{};
    bool bDrawGridLines{};
    std::optional<UVec2Int> SelectedTileCoords{};
    std::shared_ptr<SLevel> Level{};

    void Init(struct SDL_Window *Window, void *Context);

    static void Cleanup();

    static void ProcessEvent(const union SDL_Event *Event);

    void Update();

    static void DebugTools(SDebugToolsData &Data);

    void Draw() const;

    [[nodiscard]] bool IsLevelLoaded() const {
        return Level != nullptr;
    }

private:
    void DrawLevel();

    template<typename E>
    void EnumCombo(const char *Label, const char **Types, const int TypeCount, E *SelectedType) {
        if (ImGui::BeginCombo(Label, Types[(int) *SelectedType], ImGuiComboFlags_NoArrowButton)) {
            for (int I = 0; I < TypeCount; I++) {
                bool bIsSelected = I ==
                                   (int) *SelectedType;
                if (ImGui::Selectable(Types[I], bIsSelected)) {
                    *SelectedType = static_cast<E>(I);
                }
                if (bIsSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }
};