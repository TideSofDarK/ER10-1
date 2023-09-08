#include "Editor.hxx"

#include <iostream>
#include <memory>
#include "glad/gl.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl2.h"
#include "Constants.hxx"

#define BG_COLOR (ImGui::GetColorU32(IM_COL32(0, 130 / 10, 216 / 10, 255)))
#define GRID_LINE_COLOR (ImGui::GetColorU32(IM_COL32(215, 215, 215, 255)))
#define WALL_COLOR (ImGui::GetColorU32(IM_COL32(198, 205, 250, 255)))
#define WALL_JOINT_COLOR (ImGui::GetColorU32(IM_COL32(65, 205, 25, 255)))
#define FLOOR_COLOR (ImGui::GetColorU32(IM_COL32(5, 105, 205, 255)))
#define SELECTION_COLOR (ImGui::GetColorU32(IM_COL32(255, 105, 98, 255)))
#define SELECTION_MODIFY_COLOR (ImGui::GetColorU32(IM_COL32(255, 105, 200, 255)))

#define IMVEC2(SVEC) (ImVec2{SVEC.X, SVEC.Y})

void SEditor::Init(SDL_Window *Window, void *Context) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(Window, Context);
    ImGui_ImplOpenGL3_Init(GLSLVersion.c_str());
}

void SEditor::Cleanup() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void SEditor::Update() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    if (bLevelEditorActive) {
        bool bNewLevel = false;
        bool bLoadLevel = false;
        bool bSaveLevel = false;
        bool bLevelProperties = false;

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                ImGui::MenuItem("New Level", nullptr, &bNewLevel);
                ImGui::MenuItem("Load Level", nullptr, &bLoadLevel);
                ImGui::MenuItem("Save Level", nullptr, &bSaveLevel);
                ImGui::Separator();
                ImGui::MenuItem("Level Properties", nullptr, &bLevelProperties);
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("View")) {
                ImGui::MenuItem("Show Edges", nullptr, &bDrawEdges);
                ImGui::MenuItem("Show Wall Joints", nullptr, &bDrawWallJoints);
                ImGui::MenuItem("Show Grid Lines", nullptr, &bDrawGridLines);
                ImGui::EndMenu();
            }

            ImGui::Separator();

            const char *EditorModes[] = {"Normal", "Toggle Door"};
            ImGui::Text("Current Mode: %s", EditorModes[(int) LevelEditorMode]);

            ImGui::EndMainMenuBar();
        }

        if (bNewLevel) {
            ImGui::OpenPopup("New Level...");
        }
        if (ImGui::BeginPopupModal("New Level...", nullptr,
                                   ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse)) {
            ImGui::SliderInt("Width", &NewLevelSize.X, 8, MAX_LEVEL_WIDTH);
            ImGui::SliderInt("Height", &NewLevelSize.Y, 8, MAX_LEVEL_HEIGHT);
            if (ImGui::Button("Accept")) {
                Level = std::make_shared<SLevel>(SLevel{NewLevelSize.X, NewLevelSize.Y});
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        if (IsLevelLoaded()) {
            /* Validate Selected Tile */
            if (SelectedTileCoords.has_value()) {
                if (!Level->IsValidTile(*SelectedTileCoords)) {
                    SelectedTileCoords.reset();
                } else {
                    /* Tile Settings Window */
                    auto SelectedTile = Level->GetTileAtMutable(*SelectedTileCoords);
                    if (ImGui::Begin("Tile Settings", nullptr,
                                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize |
                                     ImGuiWindowFlags_NoFocusOnAppearing)) {
                        ImGui::Text("X=%d, Y=%d", SelectedTileCoords->X, SelectedTileCoords->Y);
                        ImGui::Separator();
                        const char *TileTypes[] = {"Empty", "Floor", "Hole"};
                        const char *EdgeTypes[] = {"Empty", "Wall", "Door"};
                        ImGui::BeginGroup();
                        {
                            ImGui::PushItemWidth(70.0f);

                            ImGui::BeginGroup();
                            ImGui::Dummy(ImVec2(70.0f, 0.0f));
                            ImGui::SameLine();
                            EnumCombo("##N", EdgeTypes, IM_ARRAYSIZE(EdgeTypes),
                                      &SelectedTile->Edges[(int) EDirection::North]);
                            ImGui::EndGroup();

                            ImGui::BeginGroup();
                            EnumCombo("##W", EdgeTypes, IM_ARRAYSIZE(EdgeTypes),
                                      &SelectedTile->Edges[(int) EDirection::West]);
                            ImGui::SameLine();
                            EnumCombo("##T", TileTypes, IM_ARRAYSIZE(TileTypes), &SelectedTile->Type);
                            ImGui::SameLine();
                            EnumCombo("##E", EdgeTypes, IM_ARRAYSIZE(EdgeTypes),
                                      &SelectedTile->Edges[(int) EDirection::East]);
                            ImGui::EndGroup();

                            ImGui::BeginGroup();
                            ImGui::Dummy(ImVec2(70.0f, 0.0f));
                            ImGui::SameLine();
                            EnumCombo("##S", EdgeTypes, IM_ARRAYSIZE(EdgeTypes),
                                      &SelectedTile->Edges[(int) EDirection::South]);
                            ImGui::EndGroup();

                            ImGui::PopItemWidth();

                            ImGui::EndGroup();
                        }
                    }
                }
            }

            DrawLevel();
        }
    }
}

void SEditor::DebugTools(bool *bImportLevel) {
    if (ImGui::Begin("Debug Tools", nullptr)) {
        if (ImGui::Button("Import Level From Editor")) {
            *bImportLevel = true;
        }
    }
}

void SEditor::Draw() const {
    ImGui::Render();
    ImGuiIO &io = ImGui::GetIO();
    glViewport(0, 0, (int) io.DisplaySize.x, (int) io.DisplaySize.y);
    if (bLevelEditorActive) {
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void SEditor::ProcessEvent(const SDL_Event *Event) {
    ImGui_ImplSDL2_ProcessEvent(Event);
}

void SEditor::DrawLevel() {
    ImGuiIO &IO = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(IO.DisplaySize.x * 0.5f, IO.DisplaySize.y * 0.5f), ImGuiCond_Once,
                            ImVec2(0.5f, 0.5f));
    if (ImGui::Begin("Grid", nullptr,
                     ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoSavedSettings |
                     ImGuiWindowFlags_NoTitleBar)) {
        auto *DrawList = ImGui::GetWindowDrawList();
        ImVec2 GridSize = ImVec2((float) (LevelEditorCellSize * Level->Width),
                                 (float) (LevelEditorCellSize * Level->Height));
        ImVec2 PosMin = ImGui::GetCursorScreenPos();
        ImVec2 PosMax = ImVec2(PosMin.x + GridSize.x,
                               PosMin.y + GridSize.y);

        /* Draw background */
        DrawList->AddRectFilled(PosMin, PosMax,
                                BG_COLOR);

        if (ImGui::IsWindowHovered()) {
            LevelEditorCellSize += (int) (IO.MouseWheel * 5.0f);

            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                auto MousePos = ImGui::GetMousePos();
                MousePos = ImVec2(MousePos.x - PosMin.x, MousePos.y - PosMin.y);

                int SelectedX = (int) (MousePos.x) / LevelEditorCellSize;
                int SelectedY = (int) (MousePos.y) / LevelEditorCellSize;
                SelectedTileCoords = UVec2Int{SelectedX, SelectedY};

                std::cout << SelectedX << " " << SelectedY << std::endl;
            }
        }

        /* Draw tiles */
        for (int X = 0; X <= Level->Width; X += 1) {
            for (int Y = 0; Y <= Level->Height; Y += 1) {
                auto CurrentTile = Level->GetTileAt({X, Y});
                if (CurrentTile == nullptr)
                    continue;
                auto TilePosMin = ImVec2(PosMin.x + (float) (X * LevelEditorCellSize),
                                         PosMin.y + (float) (Y * LevelEditorCellSize));
                auto TilePosMax = ImVec2(TilePosMin.x + (float) LevelEditorCellSize,
                                         TilePosMin.y + (float) LevelEditorCellSize);

                auto TileOffset = std::max(1.f, static_cast<float>(LevelEditorCellSize) / 35.0f);
                if (CurrentTile->Type == ETileType::Floor) {
                    auto EdgePosMin = TilePosMin;
                    EdgePosMin.x += TileOffset;
                    EdgePosMin.y += TileOffset;
                    auto EdgePosMax = TilePosMax;
                    EdgePosMax.x -= TileOffset;
                    EdgePosMax.y -= TileOffset;
                    DrawList->AddRectFilled(EdgePosMin, EdgePosMax,
                                            FLOOR_COLOR);
                }

                if (bDrawWallJoints) {
                    auto WallJointRadius = TileOffset;
                    if (CurrentTile->Edges[static_cast<int>(EDirection::North)] == ETileEdgeType::Wall &&
                        CurrentTile->Edges[static_cast<int>(EDirection::West)] == ETileEdgeType::Wall) {
                        DrawList->AddCircleFilled(TilePosMin, WallJointRadius, WALL_JOINT_COLOR);
                    }
                    if (CurrentTile->Edges[static_cast<int>(EDirection::North)] == ETileEdgeType::Wall &&
                        CurrentTile->Edges[static_cast<int>(EDirection::East)] == ETileEdgeType::Wall) {
                        DrawList->AddCircleFilled(ImVec2(TilePosMax.x, TilePosMin.y), WallJointRadius,
                                                  WALL_JOINT_COLOR);
                    }
                    if (CurrentTile->Edges[static_cast<int>(EDirection::South)] == ETileEdgeType::Wall &&
                        CurrentTile->Edges[static_cast<int>(EDirection::East)] == ETileEdgeType::Wall) {
                        DrawList->AddCircleFilled(TilePosMax, WallJointRadius, WALL_JOINT_COLOR);
                    }
                    if (CurrentTile->Edges[static_cast<int>(EDirection::South)] == ETileEdgeType::Wall &&
                        CurrentTile->Edges[static_cast<int>(EDirection::West)] == ETileEdgeType::Wall) {
                        DrawList->AddCircleFilled(ImVec2(TilePosMin.x, TilePosMax.y), WallJointRadius,
                                                  WALL_JOINT_COLOR);
                    }
                }
            }
        }

        /* Draw edges */
        const float EdgeThickness = 2.5f;
        const float EdgeOffset = std::min(5.f, static_cast<float>(LevelEditorCellSize) / 25.0f);
        const float DoorOffsetX = (float) LevelEditorCellSize * 0.15f;
        const float DoorOffsetY = (float) LevelEditorCellSize * 0.10f;
        if (bDrawEdges) {
            for (int Y = 0; Y <= Level->Height; Y += 1) {
                ImVec2 NorthPosMin;
                ImVec2 NorthPosMax;
                bool bNorthEdge{};

                ImVec2 SouthPosMin;
                ImVec2 SouthPosMax;
                bool bSouthEdge{};

                for (int X = 0; X <= Level->Width; X += 1) {
                    auto CurrentTile = Level->GetTileAt({X, Y});
                    if (CurrentTile == nullptr)
                        continue;
                    auto TilePosMin = ImVec2(PosMin.x + (float) (X * LevelEditorCellSize),
                                             PosMin.y + (float) (Y * LevelEditorCellSize));
                    auto TilePosMax = ImVec2(TilePosMin.x + (float) LevelEditorCellSize,
                                             TilePosMin.y + (float) LevelEditorCellSize);

                    auto bShouldDrawNorthEdge = CurrentTile->IsWallBasedEdge(EDirection::North);
                    if (bShouldDrawNorthEdge) {
                        if (!bNorthEdge) {
                            bNorthEdge = true;
                            NorthPosMin = TilePosMin;
                        }
                        NorthPosMax = ImVec2(TilePosMax.x, TilePosMin.y);
                    }
                    if ((!bShouldDrawNorthEdge || X + 1 == Level->Width) && bNorthEdge) {
                        NorthPosMin.y += EdgeOffset;
                        NorthPosMax.y += EdgeOffset;
                        NorthPosMin.x += EdgeOffset;
                        NorthPosMax.x -= EdgeOffset;
                        DrawList->AddLine(NorthPosMin, NorthPosMax, WALL_COLOR, EdgeThickness);
                        bNorthEdge = false;
                    }

                    if (CurrentTile->Edges[(int) EDirection::North] == ETileEdgeType::Door) {
                        auto DoorPosMin = ImVec2(TilePosMin.x + DoorOffsetX, TilePosMin.y + (EdgeThickness * 1.5f));
                        auto DoorPosMax = ImVec2(TilePosMax.x - DoorOffsetX, TilePosMin.y + DoorOffsetY);
                        DrawList->AddRectFilled(DoorPosMin, DoorPosMax, WALL_COLOR);
                    }

                    auto bShouldDrawSouthEdge = CurrentTile->IsWallBasedEdge(EDirection::South);
                    if (bShouldDrawSouthEdge) {
                        if (!bSouthEdge) {
                            bSouthEdge = true;
                            SouthPosMin = TilePosMin;
                            SouthPosMin.y = TilePosMax.y;
                        }
                        SouthPosMax = TilePosMax;
                    }
                    if ((!bShouldDrawSouthEdge || X + 1 == Level->Width) && bSouthEdge) {
                        SouthPosMin.y -= EdgeOffset;
                        SouthPosMax.y -= EdgeOffset;
                        SouthPosMin.x += EdgeOffset;
                        SouthPosMax.x -= EdgeOffset;
                        DrawList->AddLine(SouthPosMin, SouthPosMax, WALL_COLOR, EdgeThickness);
                        bSouthEdge = false;
                    }

                    if (CurrentTile->Edges[(int) EDirection::South] == ETileEdgeType::Door) {
                        auto DoorPosMin = ImVec2(TilePosMin.x + DoorOffsetX, TilePosMax.y - DoorOffsetY);
                        auto DoorPosMax = ImVec2(TilePosMax.x - DoorOffsetX, TilePosMax.y - (EdgeThickness * 1.5f));
                        DrawList->AddRectFilled(DoorPosMin, DoorPosMax, WALL_COLOR);
                    }
                }
            }

            for (int X = 0; X <= Level->Width; X += 1) {
                ImVec2 WestPosMin;
                ImVec2 WestPosMax;
                bool bWestEdge{};

                ImVec2 EastPosMin;
                ImVec2 EastPosMax;
                bool bEastEdge{};

                for (int Y = 0; Y <= Level->Height; Y += 1) {
                    auto CurrentTile = Level->GetTileAt({X, Y});
                    if (CurrentTile == nullptr)
                        continue;
                    auto TilePosMin = ImVec2(PosMin.x + (float) (X * LevelEditorCellSize),
                                             PosMin.y + (float) (Y * LevelEditorCellSize));
                    auto TilePosMax = ImVec2(TilePosMin.x + (float) LevelEditorCellSize,
                                             TilePosMin.y + (float) LevelEditorCellSize);

                    auto bShouldDrawWestEdge = CurrentTile->IsWallBasedEdge(EDirection::West);
                    if (bShouldDrawWestEdge) {
                        if (!bWestEdge) {
                            bWestEdge = true;
                            WestPosMin = TilePosMin;
                        }
                        WestPosMax = ImVec2(TilePosMin.x, TilePosMax.y);
                    }
                    if ((!bShouldDrawWestEdge || Y + 1 == Level->Height) && bWestEdge) {
                        WestPosMin.x += EdgeOffset;
                        WestPosMax.x += EdgeOffset;
                        WestPosMin.y += EdgeOffset;
                        WestPosMax.y -= EdgeOffset;
                        DrawList->AddLine(WestPosMin, WestPosMax, WALL_COLOR, EdgeThickness);
                        bWestEdge = false;
                    }

                    if (CurrentTile->Edges[(int) EDirection::West] == ETileEdgeType::Door) {
                        auto DoorPosMin = ImVec2(TilePosMin.x + (EdgeThickness * 1.5f), TilePosMin.y + DoorOffsetX);
                        auto DoorPosMax = ImVec2(TilePosMin.x + DoorOffsetY, TilePosMax.y - DoorOffsetX);
                        DrawList->AddRectFilled(DoorPosMin, DoorPosMax, WALL_COLOR);
                    }

                    auto bShouldDrawEastEdge = CurrentTile->IsWallBasedEdge(EDirection::East);
                    if (bShouldDrawEastEdge) {
                        if (!bEastEdge) {
                            bEastEdge = true;
                            EastPosMin = TilePosMin;
                            EastPosMin.x = TilePosMax.x;
                        }
                        EastPosMax = TilePosMax;
                    }
                    if ((!bShouldDrawEastEdge || Y + 1 == Level->Height) && bEastEdge) {
                        EastPosMin.x -= EdgeOffset;
                        EastPosMax.x -= EdgeOffset;
                        EastPosMin.y += EdgeOffset;
                        EastPosMax.y -= EdgeOffset;
                        DrawList->AddLine(EastPosMin, EastPosMax, WALL_COLOR, EdgeThickness);
                        bEastEdge = false;
                    }

                    if (CurrentTile->Edges[(int) EDirection::East] == ETileEdgeType::Door) {
                        auto DoorPosMin = ImVec2(TilePosMax.x - DoorOffsetY, TilePosMin.y + DoorOffsetX);
                        auto DoorPosMax = ImVec2(TilePosMax.x - (EdgeThickness * 1.5f), TilePosMax.y - DoorOffsetX);
                        DrawList->AddRectFilled(DoorPosMin, DoorPosMax, WALL_COLOR);
                    }
                }
            }
        }

        /* Draw grid lines */
        if (bDrawGridLines) {
            for (int X = 0; X <= (int) GridSize.x; X += LevelEditorCellSize) {
                DrawList->AddLine(ImVec2(PosMin.x + (float) X, PosMin.y), ImVec2(PosMin.x + (float) X, PosMax.y - 1),
                                  GRID_LINE_COLOR);
            }

            for (int Y = 0; Y <= (int) GridSize.y; Y += LevelEditorCellSize) {
                DrawList->AddLine(ImVec2(PosMin.x, PosMin.y + (float) Y), ImVec2(PosMax.x + 1, PosMin.y + (float) Y),
                                  GRID_LINE_COLOR);
            }
        }

        if (SelectedTileCoords.has_value()) {
            if (ImGui::IsWindowFocused()) {
                if (LevelEditorMode == ELevelEditorMode::Normal) {
                    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_UpArrow))) {
                        SelectedTileCoords->Y = std::max(0, SelectedTileCoords->Y - 1);
                    }
                    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow))) {
                        SelectedTileCoords->X = std::max(0, SelectedTileCoords->X - 1);
                    }
                    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_DownArrow))) {
                        SelectedTileCoords->Y = std::min(Level->Height - 1, SelectedTileCoords->Y + 1);
                    }
                    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow))) {
                        SelectedTileCoords->X = std::min(Level->Width - 1, SelectedTileCoords->X + 1);
                    }
                    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Space))) {
                        Level->Excavate(*SelectedTileCoords);
                    }
                    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_D))) {
                        LevelEditorMode = ELevelEditorMode::ToggleDoor;
                    }
                } else if (LevelEditorMode == ELevelEditorMode::ToggleDoor) {
                    auto SelectedTile = Level->GetTileAtMutable(*SelectedTileCoords);
                    auto ToggleDoor = [this](ETileEdgeType *TileEdgeType) {
                        if (*TileEdgeType == ETileEdgeType::Door) {
                            *TileEdgeType = ETileEdgeType::Wall;
                        } else {
                            *TileEdgeType = ETileEdgeType::Door;
                        }
                        LevelEditorMode = ELevelEditorMode::Normal;
                    };
                    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_UpArrow))) {
                        ToggleDoor(&SelectedTile->Edges[(int) EDirection::North]);
                    } else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow))) {
                        ToggleDoor(&SelectedTile->Edges[(int) EDirection::West]);
                    } else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_DownArrow))) {
                        ToggleDoor(&SelectedTile->Edges[(int) EDirection::South]);
                    } else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow))) {
                        ToggleDoor(&SelectedTile->Edges[(int) EDirection::East]);
                    }
                }
            }

            /* Outline Selected Tile */
            {
                auto SelectedTilePosMin = ImVec2(PosMin.x + (float) (SelectedTileCoords->X * LevelEditorCellSize),
                                                 PosMin.y + (float) (SelectedTileCoords->Y * LevelEditorCellSize));
                auto SelectedTilePosMax = ImVec2(SelectedTilePosMin.x + (float) LevelEditorCellSize + 1,
                                                 SelectedTilePosMin.y + (float) LevelEditorCellSize + 1);
                if (LevelEditorMode == ELevelEditorMode::Normal) {
                    DrawList->AddRect(SelectedTilePosMin, SelectedTilePosMax,
                                      SELECTION_COLOR, 0.0f, 0, 2.0f);
                } else {
                    auto Thickness = 2.0f + std::abs(2.0f * std::sin((float) ImGui::GetTime() * 10.0f));
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
}
