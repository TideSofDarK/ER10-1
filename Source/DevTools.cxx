#include "DevTools.hxx"

#include <iostream>
#include "glad/gl.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl2.h"
#include "Constants.hxx"
#include "GameSystem.hxx"

#define BG_COLOR (ImGui::GetColorU32(IM_COL32(0, 130 / 10, 216 / 10, 255)))
#define GRID_LINE_COLOR (ImGui::GetColorU32(IM_COL32(215, 215, 215, 255)))
#define WALL_COLOR (ImGui::GetColorU32(IM_COL32(198, 205, 250, 255)))
#define WALL_JOINT_COLOR (ImGui::GetColorU32(IM_COL32(65, 205, 25, 255)))
#define FLOOR_COLOR (ImGui::GetColorU32(IM_COL32(5, 105, 205, 255)))
#define SELECTION_COLOR (ImGui::GetColorU32(IM_COL32(255, 105, 98, 255)))
#define SELECTION_MODIFY_COLOR (ImGui::GetColorU32(IM_COL32(255, 105, 200, 255)))

#define PARTY_SLOT_COLOR (ImGui::GetColorU32(IM_COL32(100, 75, 230, 200)))
#define HPBAR_COLOR (ImGui::GetColorU32(IM_COL32(255, 19, 25, 255)))

void SDevTools::Init(SDL_Window* Window, void* Context)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(Window, Context);
    ImGui_ImplOpenGL3_Init(GLSLVersion.c_str());

    bLevelEditorActive = false;
    LevelEditorMode = ELevelEditorMode::Normal;
    NewLevelSize = UVec2Int{ 8, 8 };
    LevelEditorCellSize = 32.0f;
    bDrawWallJoints = false;
    bDrawEdges = true;
    bDrawGridLines = false;
    Level = SLevel{ 8, 8 };
}

void SDevTools::Cleanup()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void SDevTools::Update()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    if (bLevelEditorActive)
    {
        bool bNewLevel = false;
        bool bLoadLevel = false;
        bool bSaveLevel = false;
        bool bLevelProperties = false;

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                ImGui::MenuItem("New Level", nullptr, &bNewLevel);
                ImGui::MenuItem("Load Level", nullptr, &bLoadLevel);
                ImGui::MenuItem("Save Level", nullptr, &bSaveLevel);
                ImGui::Separator();
                ImGui::MenuItem("Level Properties", nullptr, &bLevelProperties);
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

            const char* EditorModes[] = { "Normal", "Toggle Door" };
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
                Level = SLevel{ NewLevelSize.X, NewLevelSize.Y };
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        /* Validate Selected Tile */
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
                    ImGui::BeginGroup();
                    {
                        ImGui::PushItemWidth(70.0f);

                        ImGui::BeginGroup();
                        ImGui::Dummy(ImVec2(70.0f, 0.0f));
                        ImGui::SameLine();
                        EnumCombo("##N", EdgeTypes, IM_ARRAYSIZE(EdgeTypes),
                            &SelectedTile->Edges[SDirection::North().Index]);
                        ImGui::EndGroup();

                        ImGui::BeginGroup();
                        EnumCombo("##W", EdgeTypes, IM_ARRAYSIZE(EdgeTypes),
                            &SelectedTile->Edges[SDirection::West().Index]);
                        ImGui::SameLine();
                        EnumCombo("##T", TileTypes, IM_ARRAYSIZE(TileTypes), &SelectedTile->Type);
                        ImGui::SameLine();
                        EnumCombo("##E", EdgeTypes, IM_ARRAYSIZE(EdgeTypes),
                            &SelectedTile->Edges[SDirection::East().Index]);
                        ImGui::EndGroup();

                        ImGui::BeginGroup();
                        ImGui::Dummy(ImVec2(70.0f, 0.0f));
                        ImGui::SameLine();
                        EnumCombo("##S", EdgeTypes, IM_ARRAYSIZE(EdgeTypes),
                            &SelectedTile->Edges[SDirection::South().Index]);
                        ImGui::EndGroup();

                        ImGui::PopItemWidth();

                        ImGui::EndGroup();
                    }
                    ImGui::End();
                }
            }
        }

        DrawLevel();
    }
}

void SDevTools::DebugTools(SDebugToolsData& Data)
{
    if (ImGui::Begin("Debug Tools", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        if (ImGui::TreeNode("System Info"))
        {
            ImGui::Text("Frames Per Second: %.6f", Data.FPS);
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Player Info"))
        {
            ImGui::Text("Direction: %s", SDirection::Names[Data.PlayerDirection.Index]);
            ImGui::Text("Coords: X=%d, Y=%d", Data.PlayerCoords.X, Data.PlayerCoords.Y);
            if (Data.Party)
            {
                DrawParty(*Data.Party, 0.5f, true);
                DrawParty(*Data.Party, 0.5f, false);
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Level Tools"))
        {
            if (ImGui::Button("Import Level From Editor"))
            {
                Data.bImportLevel = true;
            }
            ImGui::TreePop();
        }
        ImGui::End();
    }
}

void SDevTools::Draw() const
{
    ImGui::Render();
    ImGuiIO& io = ImGui::GetIO();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    if (bLevelEditorActive)
    {
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void SDevTools::ProcessEvent(const SDL_Event* Event)
{
    ImGui_ImplSDL2_ProcessEvent(Event);
}

void SDevTools::DrawLevel()
{
    ImGuiIO& IO = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(IO.DisplaySize.x * 0.5f, IO.DisplaySize.y * 0.5f), ImGuiCond_Once,
        ImVec2(0.5f, 0.5f));
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

        /* Draw background */
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

        /* Draw tiles */
        for (int X = 0; X <= Level.Width; X += 1)
        {
            for (int Y = 0; Y <= Level.Height; Y += 1)
            {
                auto CurrentTile = Level.GetTileAt({ X, Y });
                if (CurrentTile == nullptr)
                    continue;
                auto TilePosMin = ImVec2(PosMin.x + ((float)X * LevelEditorCellSize),
                    PosMin.y + ((float)Y * LevelEditorCellSize));
                auto TilePosMax = ImVec2(TilePosMin.x + (float)LevelEditorCellSize,
                    TilePosMin.y + (float)LevelEditorCellSize);

                auto TileOffset = (float)LevelEditorCellSize * 0.045f;
                if (CurrentTile->Type == ETileType::Floor)
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
                    if (CurrentTile->Edges[SDirection::North().Index] == ETileEdgeType::Wall && CurrentTile->Edges[SDirection::West().Index] == ETileEdgeType::Wall)
                    {
                        DrawList->AddCircleFilled(TilePosMin, WallJointRadius, WALL_JOINT_COLOR);
                    }
                    if (CurrentTile->Edges[SDirection::North().Index] == ETileEdgeType::Wall && CurrentTile->Edges[SDirection::East().Index] == ETileEdgeType::Wall)
                    {
                        DrawList->AddCircleFilled(ImVec2(TilePosMax.x, TilePosMin.y), WallJointRadius,
                            WALL_JOINT_COLOR);
                    }
                    if (CurrentTile->Edges[SDirection::South().Index] == ETileEdgeType::Wall && CurrentTile->Edges[SDirection::East().Index] == ETileEdgeType::Wall)
                    {
                        DrawList->AddCircleFilled(TilePosMax, WallJointRadius, WALL_JOINT_COLOR);
                    }
                    if (CurrentTile->Edges[SDirection::South().Index] == ETileEdgeType::Wall && CurrentTile->Edges[SDirection::West().Index] == ETileEdgeType::Wall)
                    {
                        DrawList->AddCircleFilled(ImVec2(TilePosMin.x, TilePosMax.y), WallJointRadius,
                            WALL_JOINT_COLOR);
                    }
                }
            }
        }

        /* Draw edges */
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
                        continue;
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

                    if (CurrentTile->Edges[SDirection::North().Index] == ETileEdgeType::Door)
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

                    if (CurrentTile->Edges[SDirection::South().Index] == ETileEdgeType::Door)
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
                        continue;
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

                    if (CurrentTile->Edges[SDirection::West().Index] == ETileEdgeType::Door)
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

                    if (CurrentTile->Edges[SDirection::East().Index] == ETileEdgeType::Door)
                    {
                        auto DoorPosMin = ImVec2(TilePosMax.x - DoorOffsetY, TilePosMin.y + DoorOffsetX);
                        auto DoorPosMax = ImVec2(TilePosMax.x - (EdgeThickness * 1.5f), TilePosMax.y - DoorOffsetX);
                        DrawList->AddRectFilled(DoorPosMin, DoorPosMax, WALL_COLOR);
                    }
                }
            }
        }

        /* Draw grid lines */
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

        if (SelectedTileCoords.has_value())
        {
            if (ImGui::IsWindowFocused())
            {
                if (LevelEditorMode == ELevelEditorMode::Normal)
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
                        SelectedTileCoords->Y = std::min(Level.Height - 1, SelectedTileCoords->Y + 1);
                    }
                    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow)))
                    {
                        SelectedTileCoords->X = std::min(Level.Width - 1, SelectedTileCoords->X + 1);
                    }
                    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Space)))
                    {
                        Level.Excavate(*SelectedTileCoords);
                    }
                    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_D)))
                    {
                        LevelEditorMode = ELevelEditorMode::ToggleDoor;
                    }
                }
                else if (LevelEditorMode == ELevelEditorMode::ToggleDoor)
                {
                    auto SelectedTile = Level.GetTileAtMutable(*SelectedTileCoords);
                    auto ToggleDoor = [this](ETileEdgeType* TileEdgeType) {
                        if (*TileEdgeType == ETileEdgeType::Door)
                        {
                            *TileEdgeType = ETileEdgeType::Wall;
                        }
                        else
                        {
                            *TileEdgeType = ETileEdgeType::Door;
                        }
                        LevelEditorMode = ELevelEditorMode::Normal;
                    };
                    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_UpArrow)))
                    {
                        ToggleDoor(&SelectedTile->Edges[SDirection::North().Index]);
                    }
                    else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow)))
                    {
                        ToggleDoor(&SelectedTile->Edges[SDirection::West().Index]);
                    }
                    else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_DownArrow)))
                    {
                        ToggleDoor(&SelectedTile->Edges[SDirection::South().Index]);
                    }
                    else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow)))
                    {
                        ToggleDoor(&SelectedTile->Edges[SDirection::East().Index]);
                    }
                }
            }

            /* Outline Selected Tile */
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

        ImGui::Dummy(ImVec2(1 + GridSize.x,
            1 + GridSize.y));

        //        if (!ImGui::IsWindowFocused()) {
        //            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        //                SelectedTileCoords.reset();
        //            }
        //        }

        ImGui::End();
    }
    ImGui::PopStyleVar();
}

void SDevTools::DrawParty(SParty& Party, float Scale, bool bReversed)
{
    //    if (ImGui::Begin("Player Party", nullptr, 0)) { //ImGuiWindowFlags_AlwaysAutoResize
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
                //                    DrawList->AddText(LabelPos, ImGui::GetColorU32({255,255,255,255}),  "Test", LabelPos);
            }
        }
    }

    const ImVec2 PartyBlockSize = { SlotWidth * (float)PARTY_COLS,
        SlotHeight * (float)PARTY_ROWS };
    ImGui::Dummy(PartyBlockSize);

    PosMin.y += PartyBlockSize.y;
    ImGui::SetCursorScreenPos(PosMin);
}
