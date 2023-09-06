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

#pragma region LevelEditor
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
            /* Validate selected tile */
            if (SelectedTileCoords.has_value()) {
                if (SelectedTileCoords->X > Level->Width - 1 || SelectedTileCoords->X < 0 ||
                    SelectedTileCoords->Y > Level->Height - 1 || SelectedTileCoords->Y < 0) {
                    SelectedTileCoords.reset();
                }
            }

            DrawLevel();
        }
    }
#pragma endregion
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
    ImGui::SetNextWindowPos(ImVec2(IO.DisplaySize.x * 0.5f, IO.DisplaySize.y * 0.5f), ImGuiCond_Always,
                            ImVec2(0.5f, 0.5f));
    if (ImGui::Begin("Grid", nullptr,
                     ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoSavedSettings |
                     ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_NoMove)) {
        auto *DrawList = ImGui::GetWindowDrawList();
        ImVec2 GridSize = ImVec2(LevelEditorCellSize * Level->Width, LevelEditorCellSize * Level->Height);
        ImVec2 PosMin = ImGui::GetCursorScreenPos();
        ImVec2 PosMax = ImVec2(PosMin.x + GridSize.x,
                               PosMin.y + GridSize.y);
        /* Draw grid background */
        DrawList->AddRectFilled(PosMin, PosMax,
                                BG_COLOR);


        if (ImGui::IsWindowHovered()) {
            LevelEditorCellSize += static_cast<int>(IO.MouseWheel * 5.0f);

            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                auto MousePos = ImGui::GetMousePos();
                MousePos = ImVec2(MousePos.x - PosMin.x, MousePos.y - PosMin.y);

                int SelectedX = static_cast<int>(MousePos.x) / LevelEditorCellSize;
                int SelectedY = static_cast<int>(MousePos.y) / LevelEditorCellSize;
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
                auto TilePosMin = ImVec2(PosMin.x + (X * LevelEditorCellSize),
                                         PosMin.y + (Y * LevelEditorCellSize));
                auto TilePosMax = ImVec2(TilePosMin.x + LevelEditorCellSize,
                                         TilePosMin.y + LevelEditorCellSize);

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
                    auto TilePosMin = ImVec2(PosMin.x + (X * LevelEditorCellSize),
                                             PosMin.y + (Y * LevelEditorCellSize));
                    auto TilePosMax = ImVec2(TilePosMin.x + LevelEditorCellSize,
                                             TilePosMin.y + LevelEditorCellSize);

                    auto bNorthWall = CurrentTile->Edges[static_cast<int>(EDirection::North)] == ETileEdgeType::Wall;
                    if (bNorthWall) {
                        if (!bNorthEdge) {
                            bNorthEdge = true;
                            NorthPosMin = TilePosMin;
                        }
                        NorthPosMax = ImVec2(TilePosMax.x, TilePosMin.y);
                    }
                    if ((!bNorthWall || X + 1 == Level->Width) && bNorthEdge) {
                        DrawList->AddLine(NorthPosMin, NorthPosMax, WALL_COLOR, 2.5f);
                        bNorthEdge = false;
                    }

                    auto bSouthWall = CurrentTile->Edges[static_cast<int>(EDirection::South)] == ETileEdgeType::Wall;
                    if (bSouthWall) {
                        if (!bSouthEdge) {
                            bSouthEdge = true;
                            SouthPosMin = TilePosMin;
                            SouthPosMin.y = TilePosMax.y;
                        }
                        SouthPosMax = TilePosMax;
                    }
                    if ((!bSouthWall || X + 1 == Level->Width) && bSouthEdge) {
                        DrawList->AddLine(SouthPosMin, SouthPosMax, WALL_COLOR, 2.5f);
                        bSouthEdge = false;
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
                    auto TilePosMin = ImVec2(PosMin.x + (X * LevelEditorCellSize),
                                             PosMin.y + (Y * LevelEditorCellSize));
                    auto TilePosMax = ImVec2(TilePosMin.x + LevelEditorCellSize,
                                             TilePosMin.y + LevelEditorCellSize);

                    auto bWestWall = CurrentTile->Edges[static_cast<int>(EDirection::West)] == ETileEdgeType::Wall;
                    if (bWestWall) {
                        if (!bWestEdge) {
                            bWestEdge = true;
                            WestPosMin = TilePosMin;
                        }
                        WestPosMax = ImVec2(TilePosMin.x, TilePosMax.y);
                    }
                    if ((!bWestWall || Y + 1 == Level->Height) && bWestEdge) {
                        DrawList->AddLine(WestPosMin, WestPosMax, WALL_COLOR, 2.5f);
                        bWestEdge = false;
                    }

                    auto bEastWall = CurrentTile->Edges[static_cast<int>(EDirection::East)] == ETileEdgeType::Wall;
                    if (bEastWall) {
                        if (!bEastEdge) {
                            bEastEdge = true;
                            EastPosMin = TilePosMin;
                            EastPosMin.x = TilePosMax.x;
                        }
                        EastPosMax = TilePosMax;
                    }
                    if ((!bEastWall || Y + 1 == Level->Height) && bEastEdge) {
                        DrawList->AddLine(EastPosMin, EastPosMax, WALL_COLOR, 2.5f);
                        bEastEdge = false;
                    }
                }
            }
        }

        /* Draw grid lines */
        if (bDrawGridLines) {
            for (int X = 0; X <= GridSize.x; X += LevelEditorCellSize) {
                DrawList->AddLine(ImVec2(PosMin.x + X, PosMin.y), ImVec2(PosMin.x + X, PosMax.y - 1),
                                  GRID_LINE_COLOR);
            }

            for (int Y = 0; Y <= GridSize.y; Y += LevelEditorCellSize) {
                DrawList->AddLine(ImVec2(PosMin.x, PosMin.y + Y), ImVec2(PosMax.x + 1, PosMin.y + Y),
                                  GRID_LINE_COLOR);
            }
        }

        if (SelectedTileCoords.has_value()) {
            if (ImGui::IsWindowFocused()) {
                if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_W))) {
                    SelectedTileCoords->Y = std::max(0, SelectedTileCoords->Y - 1);
                }
                if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_A))) {
                    SelectedTileCoords->X = std::max(0, SelectedTileCoords->X - 1);
                }
                if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_S))) {
                    SelectedTileCoords->Y = std::min(Level->Height - 1, SelectedTileCoords->Y + 1);
                }
                if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_D))) {
                    SelectedTileCoords->X = std::min(Level->Width - 1, SelectedTileCoords->X + 1);
                }
            }

            auto SelectedTilePosMin = ImVec2(PosMin.x + (SelectedTileCoords->X * LevelEditorCellSize),
                                             PosMin.y + (SelectedTileCoords->Y * LevelEditorCellSize));
            auto SelectedTilePosMax = ImVec2(SelectedTilePosMin.x + LevelEditorCellSize + 1,
                                             SelectedTilePosMin.y + LevelEditorCellSize + 1);
            DrawList->AddRect(SelectedTilePosMin, SelectedTilePosMax,
                              ImGui::GetColorU32(IM_COL32(255, 105, 98, 255)), 0.0f, 0, 2.0f);
        }

        ImGui::Dummy(ImVec2(1 + GridSize.x,
                            1 + GridSize.y));

        if (!ImGui::IsWindowFocused()) {
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                SelectedTileCoords.reset();
            }
        }

        ImGui::End();
    }
}