#pragma once

#include <optional>
#include <filesystem>
#include <imgui/imgui.h>
#include "Level.hxx"
#include "Draw.hxx"
#include "World.hxx"

enum class EDevToolsMode
{
    Game,
    LevelEditor,
    WorldEditor
};

struct SWorldEditor
{
    SWorld World;
    SFramebuffer MapFramebuffer;

    void Init();
    void Cleanup();

    void SetActive(struct SGame& Game, bool bActive);
    void Show(struct SGame& Game);
};

enum class ELevelEditorMode
{
    Normal,
    Block,
    ToggleEdge,
};

struct SValidationResult
{
    int Wall{};
    int Door{};
};

struct SLevelEditor
{
    ELevelEditorMode LevelEditorMode{};
    uint32_t ToggleEdgeType{};
    UVec2Int NewLevelSize{};
    float MapScale{};
    bool bDrawWallJoints{};
    bool bDrawEdges{};
    bool bDrawGridLines{};
    bool bResetGridPosition = true;
    std::optional<UVec2Int> SelectedTileCoords{};
    std::optional<UVec2Int> BlockModeTileCoords{};
    SLevel Level{};
    SFramebuffer MapFramebuffer;
    SUniformBlock MapUniformBlock;

    void Init();
    void Cleanup();

    void SetActive(struct SGame& Game, bool bActive);
    void Show(struct SGame& Game);
    void ShowLevel(struct SGame& Game);

    void SaveTilemapToFile(const std::filesystem::path& Path);
    void LoadTilemapFromFile(const std::filesystem::path& Path);

    void ScanForLevels();

    UVec2Int CalculateMapSize();
    void FitTilemapToWindow();

    SValidationResult Validate(bool bFix);
};

struct SDevTools
{
    EDevToolsMode Mode{};
    SLevelEditor LevelEditor;
    SWorldEditor WorldEditor;

    void Init(struct SDL_Window* Window, void* Context);

    void Cleanup();

    static void ProcessEvent(const union SDL_Event* Event);

    void Update(struct SGame& Game);

    void DebugTools(struct SGame& Game) const;

    static void DrawParty(struct SParty& Party, float Scale, bool bReversed);

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
