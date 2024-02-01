#pragma once

#include <optional>
#include <filesystem>
#include <imgui/imgui.h>
#include "Level.hxx"

struct SParty;

enum class ELevelEditorMode
{
    Normal,
    Block,
    ToggleDoor
};

struct SLevelEditor
{
    bool bLevelEditorActive{};
    ELevelEditorMode LevelEditorMode{};
    UVec2Int NewLevelSize{};
    float LevelEditorCellSize{};
    bool bDrawWallJoints{};
    bool bDrawEdges{};
    bool bDrawGridLines{};
    bool bResetGridPosition = true;
    std::optional<UVec2Int> SelectedTileCoords{};
    std::optional<UVec2Int> BlockModeTileCoords{};
    SLevel Level{};

    SLevelEditor();

    void Show();

    void SaveTilemapToFile(const std::filesystem::path& Path) const;
    void LoadTilemapFromFile(const std::filesystem::path& Path);

    void ScanForLevels();

    void ShowLevel();

    void FitTilemapToWindow();
};

struct SDevTools
{
    SLevelEditor LevelEditor;

    void Init(struct SDL_Window* Window, void* Context);

    static void Cleanup();

    static void ProcessEvent(const union SDL_Event* Event);

    void Update(struct SGame& Game);

    static void DrawParty(SParty& Party, float Scale, bool bReversed);

    void Draw() const;

private:
    template <typename E>
    void EnumCombo(const char* Label, const char** Types, const int TypeCount, E* SelectedType)
    {
        if (ImGui::BeginCombo(Label, Types[(int)*SelectedType], ImGuiComboFlags_NoArrowButton))
        {
            for (int I = 0; I < TypeCount; I++)
            {
                bool bIsSelected = I == (int)*SelectedType;
                if (ImGui::Selectable(Types[I], bIsSelected))
                {
                    *SelectedType = static_cast<E>(I);
                }
                if (bIsSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
    }
};
