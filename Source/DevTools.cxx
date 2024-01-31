#include "DevTools.hxx"

#include <cstdint>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <glad/gl.h>
#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_impl_sdl3.h>
#include "CommonTypes.hxx"
#include "Constants.hxx"
#include "Game.hxx"
#include "GameSystem.hxx"
#include "AssetTools.hxx"
#include "Level.hxx"
#include "Math.hxx"
#include "Memory.hxx"

#define BG_COLOR (ImGui::GetColorU32(IM_COL32(0, 130 / 10, 216 / 10, 255)))
#define GRID_LINE_COLOR (ImGui::GetColorU32(IM_COL32(215, 215, 215, 255)))
#define WALL_COLOR (ImGui::GetColorU32(IM_COL32(198, 205, 250, 255)))
#define WALL_JOINT_COLOR (ImGui::GetColorU32(IM_COL32(65, 205, 25, 255)))
#define FLOOR_COLOR (ImGui::GetColorU32(IM_COL32(5, 105, 205, 255)))
#define SELECTION_COLOR (ImGui::GetColorU32(IM_COL32(255, 105, 98, 255)))
#define SELECTION_MODIFY_COLOR (ImGui::GetColorU32(IM_COL32(255, 105, 200, 255)))
#define BLOCK_SELECTION_COLOR (ImGui::GetColorU32(IM_COL32(215, 205, 35, 255)))

#define PARTY_SLOT_COLOR (ImGui::GetColorU32(IM_COL32(100, 75, 230, 200)))
#define HPBAR_COLOR (ImGui::GetColorU32(IM_COL32(255, 19, 25, 255)))

namespace Asset::Common
{
    EXTERN_ASSET(BerkeleyTTF)
}

static const std::filesystem::path MapExtension = ".erm";
static std::vector<std::filesystem::path> AvailableMaps(20);

SLevelEditor::SLevelEditor()
{
    bLevelEditorActive = false;
    LevelEditorMode = ELevelEditorMode::Normal;
    NewLevelSize = UVec2Int{ 8, 8 };
    LevelEditorCellSize = 32.0f;
    bDrawWallJoints = false;
    bDrawEdges = true;
    bDrawGridLines = false;
    Level = SLevel{ 8, 8 };
}

void SLevelEditor::Show()
{
    bool bNewLevel = false;
    bool bLoadLevel = false;
    bool bSaveLevel = false;
    bool bLevelProperties = false;

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            ImGui::MenuItem("New Level", "Ctrl+N", &bNewLevel);
            ImGui::MenuItem("Load Level", "Ctrl+L", &bLoadLevel);
            ImGui::MenuItem("Save Level", "Ctrl+S", &bSaveLevel);
            ImGui::Separator();
            ImGui::MenuItem("Level Properties", "F5", &bLevelProperties);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            ImGui::MenuItem("Show Edges", nullptr, &bDrawEdges);
            ImGui::MenuItem("Show Wall Joints", nullptr, &bDrawWallJoints);
            ImGui::MenuItem("Show Grid Lines", nullptr, &bDrawGridLines);
            ImGui::EndMenu();
        }

        ImGui::Separator();

        const char* EditorModes[] = { "Normal", "Block Selection", "Toggle Door" };
        ImGui::Text("Current Mode: %s", EditorModes[(int)LevelEditorMode]);

        ImGui::EndMainMenuBar();
    }

    if (bNewLevel)
    {
        ImGui::OpenPopup("New Level...");
    }
    if (ImGui::BeginPopupModal("New Level...", nullptr,
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse))
    {
        ImGui::SliderInt("Width", &NewLevelSize.X, 8, MAX_LEVEL_WIDTH);
        ImGui::SliderInt("Height", &NewLevelSize.Y, 8, MAX_LEVEL_HEIGHT);
        if (ImGui::Button("Accept"))
        {
            Level = SLevel{ (uint32_t)NewLevelSize.X, (uint32_t)NewLevelSize.Y };
            FitTilemapToWindow();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    static constexpr float ModalWidth = 400.0f;
    static constexpr float ModalHeight = 200.0f;
    static std::string SavePathString{};
    if (bSaveLevel)
    {
        ImGui::OpenPopup("Save Level");
        SavePathString = (std::filesystem::current_path().parent_path().parent_path() / (std::filesystem::path("Asset\\Map\\NewMap" + MapExtension.string()))).string();
        ScanForLevels();
    }
    if (ImGui::BeginPopupModal("Save Level", nullptr,
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse))
    {
        ImGui::BeginChild("Available Levels", ImVec2(ModalWidth, ModalHeight), ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX);

        for (auto& LevelPath : AvailableMaps)
        {
            if (ImGui::Selectable(LevelPath.string().c_str(), LevelPath.string() == SavePathString))
            {
                SavePathString = LevelPath.string();
            }
        }
        ImGui::EndChild();

        ImGui::PushItemWidth(ModalWidth);
        ImGui::InputText("##SaveAs", &SavePathString);
        ImGui::PopItemWidth();

        ImGui::BeginGroup();
        if (ImGui::Button("Accept"))
        {
            auto SavePath = std::filesystem::path(SavePathString);
            if (SavePath.extension() == MapExtension)
            {
                SaveTilemapToFile(SavePath);
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndGroup();

        ImGui::EndPopup();
    }

    if (bLoadLevel)
    {
        ImGui::OpenPopup("Load Level");
        ScanForLevels();
    }
    if (ImGui::BeginPopupModal("Load Level", nullptr,
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse))
    {
        ImGui::BeginChild("Available Levels", ImVec2(ModalWidth, ModalHeight), ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX);
        static std::filesystem::path* LoadPath = nullptr;
        for (auto& LevelPath : AvailableMaps)
        {
            if (ImGui::Selectable(LevelPath.string().c_str(), &LevelPath == LoadPath))
            {
                LoadPath = &LevelPath;
            }
        }
        ImGui::EndChild();

        ImGui::BeginGroup();
        if (ImGui::Button("Accept"))
        {
            if (LoadPath != nullptr)
            {
                LoadTilemapFromFile(*LoadPath);
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndGroup();

        ImGui::EndPopup();
    }

    /* Validate selected tile. */
    if (SelectedTileCoords.has_value())
    {
        if (!Level.IsValidTile(*SelectedTileCoords))
        {
            SelectedTileCoords.reset();
        }
        else
        {
            /* Tile Settings Window */
            auto SelectedTile = Level.GetTileAtMutable(*SelectedTileCoords);
            if (ImGui::Begin("Tile Settings", nullptr,
                    ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing))
            {
                ImGui::Text("X=%d, Y=%d", SelectedTileCoords->X, SelectedTileCoords->Y);
                ImGui::Separator();
                const char* TileTypes[] = { "Empty", "Floor", "Hole" };
                const char* EdgeTypes[] = { "Empty", "Wall", "Door" };

                for (auto Direction = 0u; Direction < 4; Direction++)
                {
                    if (ImGui::TreeNode(SDirection::Names[Direction]))
                    {
                        ImGui::CheckboxFlags("Wall", &SelectedTile->EdgeFlags, STile::DirectionBit(TILE_EDGE_WALL_BIT, SDirection{ Direction }));
                        ImGui::CheckboxFlags("Door", &SelectedTile->EdgeFlags, STile::DirectionBit(TILE_EDGE_DOOR_BIT, SDirection{ Direction }));
                        ImGui::TreePop();
                        ImGui::Spacing();
                    }
                }
                ImGui::End();
            }
        }
    }

    ShowLevel();
}

void SLevelEditor::SaveTilemapToFile(const class std::filesystem::path& Path) const
{
    const auto& Tilemap = Level;

    std::ofstream TilemapFile;
    TilemapFile.open(Path, std::ofstream::binary);
    Tilemap.Serialize(TilemapFile);
    TilemapFile.close();
}

void SLevelEditor::LoadTilemapFromFile(const std::filesystem::path& Path)
{
    auto& Tilemap = Level;
    std::ifstream TilemapFile;
    TilemapFile.open(Path, std::ifstream::binary);
    Tilemap.Deserialize(TilemapFile);
    TilemapFile.close();
    FitTilemapToWindow();
}

void SLevelEditor::ScanForLevels()
{
    namespace fs = std::filesystem;
    AvailableMaps.clear();
    auto CWD = fs::current_path().parent_path().parent_path() / "Asset\\Map";
    for (const auto& File : fs::recursive_directory_iterator(CWD))
    {
        if (!File.is_regular_file())
        {
            continue;
        }
        if (File.path().extension() != MapExtension)
        {
            continue;
        }
        AvailableMaps.emplace_back(File);
    }
}

void SLevelEditor::FitTilemapToWindow()
{
    auto Viewport = ImGui::GetMainViewport();
    LevelEditorCellSize = std::min((float)Viewport->Size.x / (float)Level.Width, (float)Viewport->Size.y / (float)Level.Height) * 0.8f;
    bResetGridPosition = true;
}

void SLevelEditor::ShowLevel()
{
    ImGuiIO& IO = ImGui::GetIO();
    const float WindowPadding = (float)LevelEditorCellSize * 0.1f;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(WindowPadding, WindowPadding));
    if (ImGui::Begin("Grid", nullptr,
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar))
    {
        auto* DrawList = ImGui::GetWindowDrawList();
        ImVec2 GridSize = ImVec2((LevelEditorCellSize * (float)Level.Width),
            (LevelEditorCellSize * (float)Level.Height));
        ImVec2 PosMin = ImGui::GetCursorScreenPos();
        ImVec2 PosMax = ImVec2(PosMin.x + GridSize.x,
            PosMin.y + GridSize.y);

        /* Draw background. */
        DrawList->AddRectFilled(PosMin, PosMax,
            BG_COLOR);

        if (ImGui::IsWindowHovered())
        {
            LevelEditorCellSize += IO.MouseWheel * 2.0f;

            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                auto MousePos = ImGui::GetMousePos();
                MousePos = ImVec2(MousePos.x - PosMin.x, MousePos.y - PosMin.y);
                SelectedTileCoords = UVec2Int{ (int)(MousePos.x / LevelEditorCellSize),
                    (int)(MousePos.y / LevelEditorCellSize) };
            }
        }

        /* Draw tiles. */
        for (int X = 0; X <= Level.Width; X += 1)
        {
            for (int Y = 0; Y <= Level.Height; Y += 1)
            {
                auto CurrentTile = Level.GetTileAt({ X, Y });
                if (CurrentTile == nullptr)
                {
                    continue;
                }
                auto TilePosMin = ImVec2(PosMin.x + ((float)X * LevelEditorCellSize),
                    PosMin.y + ((float)Y * LevelEditorCellSize));
                auto TilePosMax = ImVec2(TilePosMin.x + (float)LevelEditorCellSize,
                    TilePosMin.y + (float)LevelEditorCellSize);

                auto TileOffset = (float)LevelEditorCellSize * 0.045f;
                if (CurrentTile->CheckFlag(TILE_FLOOR_BIT))
                {
                    auto EdgePosMin = TilePosMin;
                    EdgePosMin.x += TileOffset;
                    EdgePosMin.y += TileOffset;
                    auto EdgePosMax = TilePosMax;
                    EdgePosMax.x -= TileOffset;
                    EdgePosMax.y -= TileOffset;
                    DrawList->AddRectFilled(EdgePosMin, EdgePosMax,
                        FLOOR_COLOR);
                }

                if (bDrawWallJoints)
                {
                    auto WallJointRadius = TileOffset;
                    if (CurrentTile->CheckEdgeFlag(TILE_EDGE_WALL_BIT, SDirection::North()) && CurrentTile->CheckEdgeFlag(TILE_EDGE_WALL_BIT, SDirection::West()))
                    {
                        DrawList->AddCircleFilled(TilePosMin, WallJointRadius, WALL_JOINT_COLOR);
                    }
                    if (CurrentTile->CheckEdgeFlag(TILE_EDGE_WALL_BIT, SDirection::North()) && CurrentTile->CheckEdgeFlag(TILE_EDGE_WALL_BIT, SDirection::East()))
                    {
                        DrawList->AddCircleFilled(ImVec2(TilePosMax.x, TilePosMin.y), WallJointRadius,
                            WALL_JOINT_COLOR);
                    }
                    if (CurrentTile->CheckEdgeFlag(TILE_EDGE_WALL_BIT, SDirection::South()) && CurrentTile->CheckEdgeFlag(TILE_EDGE_WALL_BIT, SDirection::East()))
                    {
                        DrawList->AddCircleFilled(TilePosMax, WallJointRadius, WALL_JOINT_COLOR);
                    }
                    if (CurrentTile->CheckEdgeFlag(TILE_EDGE_WALL_BIT, SDirection::South()) && CurrentTile->CheckEdgeFlag(TILE_EDGE_WALL_BIT, SDirection::West()))
                    {
                        DrawList->AddCircleFilled(ImVec2(TilePosMin.x, TilePosMax.y), WallJointRadius,
                            WALL_JOINT_COLOR);
                    }
                }
            }
        }

        /* Draw edges. */
        const float EdgeThickness = (float)LevelEditorCellSize * 0.03f;
        const float EdgeOffset = (float)LevelEditorCellSize * 0.049f;
        const float DoorOffsetX = (float)LevelEditorCellSize * 0.15f;
        const float DoorOffsetY = (float)LevelEditorCellSize * 0.12f;
        if (bDrawEdges)
        {
            for (int Y = 0; Y <= Level.Height; Y += 1)
            {
                ImVec2 NorthPosMin;
                ImVec2 NorthPosMax;
                bool bNorthEdge{};

                ImVec2 SouthPosMin;
                ImVec2 SouthPosMax;
                bool bSouthEdge{};

                for (int X = 0; X <= Level.Width; X += 1)
                {
                    auto CurrentTile = Level.GetTileAt({ X, Y });
                    if (CurrentTile == nullptr)
                    {
                        continue;
                    }
                    auto TilePosMin = ImVec2(PosMin.x + ((float)X * LevelEditorCellSize),
                        PosMin.y + ((float)Y * LevelEditorCellSize));
                    auto TilePosMax = ImVec2(TilePosMin.x + (float)LevelEditorCellSize,
                        TilePosMin.y + (float)LevelEditorCellSize);

                    auto bShouldDrawNorthEdge = CurrentTile->IsWallBasedEdge(SDirection::North());
                    if (bShouldDrawNorthEdge)
                    {
                        if (!bNorthEdge)
                        {
                            bNorthEdge = true;
                            NorthPosMin = TilePosMin;
                        }
                        NorthPosMax = ImVec2(TilePosMax.x, TilePosMin.y);
                    }
                    if ((!bShouldDrawNorthEdge || X + 1 == Level.Width) && bNorthEdge)
                    {
                        NorthPosMin.y += EdgeOffset;
                        NorthPosMax.y += EdgeOffset;
                        NorthPosMin.x += EdgeOffset;
                        NorthPosMax.x -= EdgeOffset;
                        DrawList->AddLine(NorthPosMin, NorthPosMax, WALL_COLOR, EdgeThickness);
                        bNorthEdge = false;
                    }

                    if (CurrentTile->CheckEdgeFlag(TILE_EDGE_DOOR_BIT, SDirection::North()))
                    {
                        auto DoorPosMin = ImVec2(TilePosMin.x + DoorOffsetX, TilePosMin.y + (EdgeThickness * 1.5f));
                        auto DoorPosMax = ImVec2(TilePosMax.x - DoorOffsetX, TilePosMin.y + DoorOffsetY);
                        DrawList->AddRectFilled(DoorPosMin, DoorPosMax, WALL_COLOR);
                    }

                    auto bShouldDrawSouthEdge = CurrentTile->IsWallBasedEdge(SDirection::South());
                    if (bShouldDrawSouthEdge)
                    {
                        if (!bSouthEdge)
                        {
                            bSouthEdge = true;
                            SouthPosMin = TilePosMin;
                            SouthPosMin.y = TilePosMax.y;
                        }
                        SouthPosMax = TilePosMax;
                    }
                    if ((!bShouldDrawSouthEdge || X + 1 == Level.Width) && bSouthEdge)
                    {
                        SouthPosMin.y -= EdgeOffset;
                        SouthPosMax.y -= EdgeOffset;
                        SouthPosMin.x += EdgeOffset;
                        SouthPosMax.x -= EdgeOffset;
                        DrawList->AddLine(SouthPosMin, SouthPosMax, WALL_COLOR, EdgeThickness);
                        bSouthEdge = false;
                    }

                    if (CurrentTile->CheckEdgeFlag(TILE_EDGE_DOOR_BIT, SDirection::South()))
                    {
                        auto DoorPosMin = ImVec2(TilePosMin.x + DoorOffsetX, TilePosMax.y - DoorOffsetY);
                        auto DoorPosMax = ImVec2(TilePosMax.x - DoorOffsetX, TilePosMax.y - (EdgeThickness * 1.5f));
                        DrawList->AddRectFilled(DoorPosMin, DoorPosMax, WALL_COLOR);
                    }
                }
            }

            for (int X = 0; X <= Level.Width; X += 1)
            {
                ImVec2 WestPosMin;
                ImVec2 WestPosMax;
                bool bWestEdge{};

                ImVec2 EastPosMin;
                ImVec2 EastPosMax;
                bool bEastEdge{};

                for (int Y = 0; Y <= Level.Height; Y += 1)
                {
                    auto CurrentTile = Level.GetTileAt({ X, Y });
                    if (CurrentTile == nullptr)
                    {
                        continue;
                    }
                    auto TilePosMin = ImVec2(PosMin.x + ((float)X * LevelEditorCellSize),
                        PosMin.y + ((float)Y * LevelEditorCellSize));
                    auto TilePosMax = ImVec2(TilePosMin.x + (float)LevelEditorCellSize,
                        TilePosMin.y + (float)LevelEditorCellSize);

                    auto bShouldDrawWestEdge = CurrentTile->IsWallBasedEdge(SDirection::West());
                    if (bShouldDrawWestEdge)
                    {
                        if (!bWestEdge)
                        {
                            bWestEdge = true;
                            WestPosMin = TilePosMin;
                        }
                        WestPosMax = ImVec2(TilePosMin.x, TilePosMax.y);
                    }
                    if ((!bShouldDrawWestEdge || Y + 1 == Level.Height) && bWestEdge)
                    {
                        WestPosMin.x += EdgeOffset;
                        WestPosMax.x += EdgeOffset;
                        WestPosMin.y += EdgeOffset;
                        WestPosMax.y -= EdgeOffset;
                        DrawList->AddLine(WestPosMin, WestPosMax, WALL_COLOR, EdgeThickness);
                        bWestEdge = false;
                    }

                    if (CurrentTile->CheckEdgeFlag(TILE_EDGE_DOOR_BIT, SDirection::West()))
                    {
                        auto DoorPosMin = ImVec2(TilePosMin.x + (EdgeThickness * 1.5f), TilePosMin.y + DoorOffsetX);
                        auto DoorPosMax = ImVec2(TilePosMin.x + DoorOffsetY, TilePosMax.y - DoorOffsetX);
                        DrawList->AddRectFilled(DoorPosMin, DoorPosMax, WALL_COLOR);
                    }

                    auto bShouldDrawEastEdge = CurrentTile->IsWallBasedEdge(SDirection::East());
                    if (bShouldDrawEastEdge)
                    {
                        if (!bEastEdge)
                        {
                            bEastEdge = true;
                            EastPosMin = TilePosMin;
                            EastPosMin.x = TilePosMax.x;
                        }
                        EastPosMax = TilePosMax;
                    }
                    if ((!bShouldDrawEastEdge || Y + 1 == Level.Height) && bEastEdge)
                    {
                        EastPosMin.x -= EdgeOffset;
                        EastPosMax.x -= EdgeOffset;
                        EastPosMin.y += EdgeOffset;
                        EastPosMax.y -= EdgeOffset;
                        DrawList->AddLine(EastPosMin, EastPosMax, WALL_COLOR, EdgeThickness);
                        bEastEdge = false;
                    }

                    if (CurrentTile->CheckEdgeFlag(TILE_EDGE_DOOR_BIT, SDirection::East()))
                    {
                        auto DoorPosMin = ImVec2(TilePosMax.x - DoorOffsetY, TilePosMin.y + DoorOffsetX);
                        auto DoorPosMax = ImVec2(TilePosMax.x - (EdgeThickness * 1.5f), TilePosMax.y - DoorOffsetX);
                        DrawList->AddRectFilled(DoorPosMin, DoorPosMax, WALL_COLOR);
                    }
                }
            }
        }

        /* Draw grid lines. */
        if (bDrawGridLines)
        {
            for (int X = 0; X <= (int)GridSize.x; X += (int)LevelEditorCellSize)
            {
                DrawList->AddLine(ImVec2(PosMin.x + (float)X, PosMin.y), ImVec2(PosMin.x + (float)X, PosMax.y - 1),
                    GRID_LINE_COLOR);
            }

            for (int Y = 0; Y <= (int)GridSize.y; Y += (int)LevelEditorCellSize)
            {
                DrawList->AddLine(ImVec2(PosMin.x, PosMin.y + (float)Y), ImVec2(PosMax.x + 1, PosMin.y + (float)Y),
                    GRID_LINE_COLOR);
            }
        }

        if (ImGui::IsWindowFocused())
        {
            if (SelectedTileCoords.has_value())
            {
                if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
                {
                    LevelEditorMode = ELevelEditorMode::Normal;
                }
                else if (LevelEditorMode == ELevelEditorMode::Normal || LevelEditorMode == ELevelEditorMode::Block)
                {
                    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_UpArrow)))
                    {
                        SelectedTileCoords->Y = std::max(0, SelectedTileCoords->Y - 1);
                    }
                    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow)))
                    {
                        SelectedTileCoords->X = std::max(0, SelectedTileCoords->X - 1);
                    }
                    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_DownArrow)))
                    {
                        SelectedTileCoords->Y = std::min((int)(Level.Height - 1), SelectedTileCoords->Y + 1);
                    }
                    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow)))
                    {
                        SelectedTileCoords->X = std::min((int)(Level.Width - 1), SelectedTileCoords->X + 1);
                    }
                }
                if (LevelEditorMode == ELevelEditorMode::Block)
                {
                    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Space)))
                    {
                        Level.EditBlock(URectInt::FromTwo(*SelectedTileCoords, *BlockModeTileCoords), TILE_FLOOR_BIT);
                        LevelEditorMode = ELevelEditorMode::Normal;
                    }
                    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_C)))
                    {
                        Level.EditBlock(URectInt::FromTwo(*SelectedTileCoords, *BlockModeTileCoords), 0);
                        LevelEditorMode = ELevelEditorMode::Normal;
                    }
                    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_H)))
                    {
                        Level.EditBlock(URectInt::FromTwo(*SelectedTileCoords, *BlockModeTileCoords), TILE_HOLE_BIT);
                        LevelEditorMode = ELevelEditorMode::Normal;
                    }
                }
                else if (LevelEditorMode == ELevelEditorMode::Normal)
                {
                    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Space)))
                    {
                        Level.Edit(*SelectedTileCoords, TILE_FLOOR_BIT);
                    }
                    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_C)))
                    {
                        Level.Edit(*SelectedTileCoords, 0);
                    }
                    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_H)))
                    {
                        Level.Edit(*SelectedTileCoords, TILE_HOLE_BIT);
                    }
                    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_D)))
                    {
                        LevelEditorMode = ELevelEditorMode::ToggleDoor;
                    }
                    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_V)))
                    {
                        LevelEditorMode = ELevelEditorMode::Block;
                        BlockModeTileCoords.emplace(*SelectedTileCoords);
                    }
                }
                else if (LevelEditorMode == ELevelEditorMode::ToggleDoor)
                {
                    auto SelectedTile = Level.GetTileAtMutable(*SelectedTileCoords);
                    auto ToggleDoor = [this](SDirection Direction) {
                        Level.ToggleEdge(*SelectedTileCoords, Direction, TILE_EDGE_DOOR_BIT);
                        LevelEditorMode = ELevelEditorMode::Normal;
                    };
                    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_UpArrow)))
                    {
                        ToggleDoor(SDirection::North());
                    }
                    else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow)))
                    {
                        ToggleDoor(SDirection::West());
                    }
                    else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_DownArrow)))
                    {
                        ToggleDoor(SDirection::South());
                    }
                    else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow)))
                    {
                        ToggleDoor(SDirection::East());
                    }
                }

                /* Outline block selection. */
                if (LevelEditorMode == ELevelEditorMode::Block && BlockModeTileCoords.has_value())
                {
                    auto MinTileX = std::min(BlockModeTileCoords->X, SelectedTileCoords->X);
                    auto MinTileY = std::min(BlockModeTileCoords->Y, SelectedTileCoords->Y);

                    auto MaxTileX = std::max(BlockModeTileCoords->X, SelectedTileCoords->X);
                    auto MaxTileY = std::max(BlockModeTileCoords->Y, SelectedTileCoords->Y);

                    auto BlockWidth = MaxTileX - MinTileX + 1;
                    auto BlockHeight = MaxTileY - MinTileY + 1;

                    auto SelectedTilePosMin = ImVec2(
                        PosMin.x + ((float)MinTileX * LevelEditorCellSize),
                        PosMin.y + ((float)MinTileY * LevelEditorCellSize));
                    auto SelectedTilePosMax = ImVec2(
                        SelectedTilePosMin.x + ((float)LevelEditorCellSize * (float)BlockWidth),
                        SelectedTilePosMin.y + ((float)LevelEditorCellSize * (float)BlockHeight));

                    auto Thickness = (float)LevelEditorCellSize * 0.04f;

                    DrawList->AddRect(SelectedTilePosMin, SelectedTilePosMax,
                        BLOCK_SELECTION_COLOR, 0.0f, 0, Thickness);
                }
                else /* Outline single tile. */
                {
                    auto SelectedTilePosMin = ImVec2(PosMin.x + ((float)SelectedTileCoords->X * LevelEditorCellSize),
                        PosMin.y + ((float)SelectedTileCoords->Y * LevelEditorCellSize));
                    auto SelectedTilePosMax = ImVec2(SelectedTilePosMin.x + (float)LevelEditorCellSize + 1,
                        SelectedTilePosMin.y + (float)LevelEditorCellSize + 1);
                    auto Thickness = (float)LevelEditorCellSize * 0.04f;
                    if (LevelEditorMode == ELevelEditorMode::Normal)
                    {
                        DrawList->AddRect(SelectedTilePosMin, SelectedTilePosMax,
                            SELECTION_COLOR, 0.0f, 0, Thickness);
                    }
                    else
                    {
                        Thickness += std::abs(1.0f * std::sin((float)ImGui::GetTime() * 10.0f));
                        DrawList->AddRect(SelectedTilePosMin, SelectedTilePosMax,
                            SELECTION_MODIFY_COLOR, 0.0f, 0, Thickness);
                    }
                }
            }
        }

        ImGui::Dummy(ImVec2(1 + GridSize.x, 1 + GridSize.y));

        // if (!ImGui::IsWindowFocused())
        // {
        //     if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        //     {
        //         SelectedTileCoords.reset();
        //     }
        // }

        if (bResetGridPosition)
        {
            ImGui::SetWindowPos(
                ImVec2(
                    IO.DisplaySize.x * 0.5f - GridSize.x * 0.5f,
                    IO.DisplaySize.y * 0.5f - GridSize.y * 0.5f));
            bResetGridPosition = false;
        }

        ImGui::End();
    }
    ImGui::PopStyleVar();

    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F5)))
    {
        FitTilemapToWindow();
    }
}

void SDevTools::DrawParty(SParty& Party, float Scale, bool bReversed)
{
    // if (ImGui::Begin("Player Party", nullptr, 0)) { //ImGuiWindowFlags_AlwaysAutoResize
    auto* DrawList = ImGui::GetWindowDrawList();
    ImVec2 PosMin = ImGui::GetCursorScreenPos();

    Scale *= ImGui::GetFontSize();
    const float SlotWidth = Scale * 15.f;
    const float SlotHeight = Scale * 9.f;
    const float SlotOffset = Scale * 1.f;

    for (int Index = 0; Index < PARTY_SIZE; ++Index)
    {
        auto& Slot = Party.Slots[Index];

        auto CurrentCol = Index % PARTY_COLS;
        auto CurrentRow = Index / PARTY_COLS;

        if (Slot.IsRealChar())
        {
            auto Char = Slot.GetRealChar();

            auto SlotPosMin = ImVec2((float)CurrentCol * SlotWidth,
                (float)CurrentRow * SlotHeight);
            SlotPosMin.x += PosMin.x;
            SlotPosMin.y += PosMin.y;

            ImVec2 SlotSize{};
            switch (Char.Size)
            {
                case 1:
                    SlotSize.x += SlotWidth;
                    SlotSize.y += SlotHeight;
                    break;
                case 2:
                    if (Char.bHorizontal)
                    {
                        SlotSize.x += SlotWidth * 2;
                        SlotSize.y += SlotHeight;
                    }
                    else
                    {
                        SlotSize.x += SlotWidth;
                        SlotSize.y += SlotHeight * 2;
                    }
                    break;
                case 4:
                    SlotSize.x += SlotWidth * 2;
                    SlotSize.y += SlotHeight * 2;
                    break;
                default:
                    ImGui::End();
                    return;
            }
            ImVec2 SlotPosMax = SlotPosMin;
            SlotPosMax.x += SlotSize.x;
            SlotPosMax.y += SlotSize.y;

            SlotPosMin.x += SlotOffset;
            SlotPosMin.y += SlotOffset;

            auto ButtonSize = SlotSize;
            ButtonSize.x -= SlotOffset;
            ButtonSize.y -= SlotOffset;

            /* Slot Button */
            {
                //                ImGui::SetCursorScreenPos(SlotPosMin);
                //                if (ImGui::Button(Char.Name, ButtonSize)) {
                //                }

                DrawList->AddRectFilled(SlotPosMin, SlotPosMax, PARTY_SLOT_COLOR);
            }

            /* HPBar */
            {
                const auto HPRatio = Char.Health / Char.MaxHealth;
                const auto HPBarHeight = SlotWidth * 0.1f;
                const auto HPBarOffsetX = SlotWidth * 0.05f;
                ImVec2 HPBarPosMin = SlotPosMin;
                HPBarPosMin.x += HPBarOffsetX;
                HPBarPosMin.y = SlotPosMax.y - HPBarOffsetX - HPBarHeight;
                ImVec2 HPBarPosMax = ImVec2(SlotPosMax.x - HPBarOffsetX, SlotPosMax.y - HPBarOffsetX);

                DrawList->AddRect(HPBarPosMin, HPBarPosMax, HPBAR_COLOR);
                HPBarPosMax.x = HPBarPosMin.x + ((ButtonSize.x - (HPBarOffsetX * 2.0f)) * HPRatio);
                DrawList->AddRectFilled(HPBarPosMin, HPBarPosMax, HPBAR_COLOR);

                auto LabelPos = ImVec2(HPBarPosMin.x, HPBarPosMin.y - ImGui::GetFontSize());
                ImGui::SetCursorScreenPos(LabelPos);
                ImGui::Text("%.0f/%.0f", Char.Health, Char.MaxHealth);

                LabelPos.y -= ImGui::GetFontSize();
                ImGui::SetCursorScreenPos(LabelPos);
                ImGui::Text("%s", Char.Name);
                // DrawList->AddText(LabelPos, ImGui::GetColorU32({255,255,255,255}), "Test", LabelPos);
            }
        }
    }

    const ImVec2 PartyBlockSize = { SlotWidth * (float)PARTY_COLS,
        SlotHeight * (float)PARTY_ROWS };
    ImGui::Dummy(PartyBlockSize);

    PosMin.y += PartyBlockSize.y;
    ImGui::SetCursorScreenPos(PosMin);
}

void SDevTools::Init(SDL_Window* Window, void* Context)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL3_InitForOpenGL(Window, Context);
    ImGui_ImplOpenGL3_Init(GLSLVersion.c_str());

    /* Don't transfer asset ownership to ImGui, it will crash otherwise! */
    ImFontConfig FontConfig;
    FontConfig.FontDataOwnedByAtlas = false;
    io.Fonts->AddFontFromMemoryTTF(Asset::Common::BerkeleyTTF.AsVoidPtr(), (int)Asset::Common::BerkeleyTTF.Length, 32.0f, &FontConfig);
}

void SDevTools::Cleanup()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

void SDevTools::Update(SGame& Game)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F9)))
    {
        LevelEditor.bLevelEditorActive = !LevelEditor.bLevelEditorActive;
        static bool bFirstTime = true;
        if (bFirstTime && LevelEditor.bLevelEditorActive)
        {
            LevelEditor.FitTilemapToWindow();
            bFirstTime = false;
        }
    }

    if (LevelEditor.bLevelEditorActive)
    {
        LevelEditor.Show();
    }
    else
    {
        if (ImGui::Begin("Debug Tools", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            if (ImGui::TreeNode("System Info"))
            {
                ImGui::Text("Frames Per Second: %.2f", 1000.0f / Game.Window.DeltaTime / 1000.0f);
                ImGui::Text("Number Of Blocks: %zu", CMemory::NumberOfBlocks());
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Player Info"))
            {
                ImGui::Text("Direction: %s (%u)", SDirection::Names[Game.Blob.Direction.Index], Game.Blob.Direction.Index);
                ImGui::Text("Coords: X=%d, Y=%d", Game.Blob.Coords.X, Game.Blob.Coords.Y);
                ImGui::Text("Unreliable Coords: X=%f, Y=%f", Game.Blob.UnreliableCoords().X, Game.Blob.UnreliableCoords().Y);
                DrawParty(Game.PlayerParty, 0.5f, true);
                DrawParty(Game.PlayerParty, 0.5f, false);
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Level Tools"))
            {
                if (ImGui::Button("Explore Level"))
                {
                    for (auto X = 0; X < Game.Level.Width; X++)
                    {
                        for (auto Y = 0; Y < Game.Level.Height; Y++)
                        {
                            auto Tile = Game.Level.GetTileAtMutable({ X, Y });
                            if (Tile != nullptr)
                            {
                                Tile->SetSpecialFlag(TILE_SPECIAL_EXPLORED_BIT);
                            }
                        }
                    }
                    Game.Level.DirtyFlags = ELevelDirtyFlags::All;
                    Game.Level.DirtyRange = { 0, Game.Level.TileCount() };
                }
                if (ImGui::Button("Visit Level"))
                {
                    for (auto X = 0; X < Game.Level.Width; X++)
                    {
                        for (auto Y = 0; Y < Game.Level.Height; Y++)
                        {
                            auto Tile = Game.Level.GetTileAtMutable({ X, Y });
                            if (Tile != nullptr)
                            {
                                Tile->SetSpecialFlag(TILE_SPECIAL_EXPLORED_BIT);
                                Tile->SetSpecialFlag(TILE_SPECIAL_VISITED_BIT);
                            }
                        }
                    }
                    Game.Level.DirtyFlags = ELevelDirtyFlags::All;
                    Game.Level.DirtyRange = { 0, Game.Level.TileCount() };
                }
                if (ImGui::Button("Import Level From Editor"))
                {
                    Game.ChangeLevel(LevelEditor.Level);
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Adjustments"))
            {
                ImGui::SliderFloat(" ", &Game.Blob.InputBufferTime, 0.0f, 1.0f, "Input Buffer Time: %.3f");
                ImGui::TreePop();
            }
            ImGui::End();
        }
    }
}

void SDevTools::Draw() const
{
    ImGui::Render();
    ImGuiIO& io = ImGui::GetIO();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    if (LevelEditor.bLevelEditorActive)
    {
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void SDevTools::ProcessEvent(const SDL_Event* Event)
{
    ImGui_ImplSDL3_ProcessEvent(Event);
}
