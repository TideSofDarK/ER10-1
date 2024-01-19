#pragma once

#include "imgui/imgui.h"
#include "Level.hxx"

struct SParty;

enum class ELevelEditorMode
{
    Normal,
    ToggleDoor
};

struct SDebugToolsData
{
    float FPS{};
    std::size_t NumberOfBlocks{};
    UVec2Int PlayerCoords{};
    SDirection PlayerDirection{ 0 };
    SParty* Party{};
    bool bImportLevel{};
    float* BufferTime{};
};

struct SDevTools
{
    bool bLevelEditorActive{};
    ELevelEditorMode LevelEditorMode{};
    UVec2Int NewLevelSize{};
    float LevelEditorCellSize{};
    bool bDrawWallJoints{};
    bool bDrawEdges{};
    bool bDrawGridLines{};
    std::optional<UVec2Int> SelectedTileCoords{};
    SLevel Level{};

    void Init(struct SDL_Window* Window, void* Context);

    static void Cleanup();

    static void ProcessEvent(const union SDL_Event* Event);

    void Update();

    static void DebugTools(SDebugToolsData& Data);

    static void DrawParty(SParty& Party, float Scale, bool bReversed);

    void Draw() const;

private:
    void DrawLevel();

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
