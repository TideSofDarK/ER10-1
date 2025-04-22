#include "DevTools.hxx"

#include <fstream>
#include <glad/gl.h>
#include <SDL3/SDL.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/imgui_stdlib.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_impl_sdl3.h>
#include "CommonTypes.hxx"
#include "Constants.hxx"
#include "Draw.hxx"
#include "Game.hxx"
#include "GameSystem.hxx"
#include "AssetTools.hxx"
#include "Utility.hxx"
#include "World.hxx"
#include "Log.hxx"
#include "Math.hxx"
#include "Memory.hxx"

#define PARTY_SLOT_COLOR (ImGui::GetColorU32(IM_COL32(100, 75, 230, 200)))
#define HPBAR_COLOR (ImGui::GetColorU32(IM_COL32(255, 19, 25, 255)))

namespace Asset::Common
{
    EXTERN_ASSET(MartianMono)
}

void SEditorFramebuffer::Init(int InWidth, int InHeight, SVec4 InClearColor)
{
    Width = InWidth;
    Height = InHeight;
    ClearColor = InClearColor;

    // glActiveTexture(0);
    glGenTextures(1, &ColorID);
    glBindTexture(GL_TEXTURE_2D, ColorID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, InWidth, InHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, ColorID, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void SEditorFramebuffer::Resize(int InWidth, int InHeight)
{
    Width = InWidth;
    Height = InHeight;
    glBindTexture(GL_TEXTURE_2D, ColorID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, InWidth, InHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    Log::DevTools<ELogLevel::Info>("Resizing editor framebuffer: Width = %d, Height = %d", InWidth, InHeight);
}

void SEditorFramebuffer::Cleanup()
{
    glDeleteFramebuffers(1, &FBO);
    glDeleteTextures(1, &ColorID);
}

static const std::filesystem::path MapExtension = ".erm";
static std::vector<std::filesystem::path> AvailableMaps(20);

static void GenericEditorWindow(
    const char* WindowName,
    SEditorBase* Editor)
{
    ImGuiIO& IO = ImGui::GetIO();

    static bool bFirstTime = true;
    if (bFirstTime)
    {
        bFirstTime = false;
    }

    auto Viewport = ImGui::GetMainViewport();
    ImVec2 WindowSize = Viewport->Size;
    ImVec2 WindowPos = ImVec2((WindowSize.x - (WindowSize.x * 0.75f)) * 0.5f, ((WindowSize.y - (WindowSize.y * 0.75f)) * 0.5f));
    WindowSize.x *= 0.75f;
    WindowSize.y *= 0.75f;

    ImGui::SetNextWindowSize(WindowSize, ImGuiCond_Once);
    ImGui::SetNextWindowPos(WindowPos, ImGuiCond_Once);
    if (ImGui::Begin(WindowName, nullptr,
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar))
    {
        Editor->EditorTools();
        ImGui::SameLine();
        if (ImGui::BeginChild("MapCanvas", ImVec2(), ImGuiChildFlags_None, ImGuiWindowFlags_NoNav))
        {
            SEditorFramebuffer* Framebuffer = &Editor->Framebuffer;
            ImVec2 CanvasSize = ImGui::GetWindowSize();
            float CanvasScale = Editor->Scale;

            SVec2Int ScaledSize{ static_cast<int32_t>(CanvasSize.x / CanvasScale), static_cast<int32_t>(CanvasSize.y / CanvasScale) };
            if (ScaledSize.X > Framebuffer->Width || ScaledSize.Y > Framebuffer->Height)
            {
                auto MaxFramebufferDimension = std::max(Utility::NextPowerOfTwo(ScaledSize.X), Utility::NextPowerOfTwo(ScaledSize.Y));
                Framebuffer->Resize((int)MaxFramebufferDimension, (int)MaxFramebufferDimension);
            }

            glBindFramebuffer(GL_FRAMEBUFFER, Framebuffer->FBO);
            glViewport(0, 0, ScaledSize.X, ScaledSize.Y);
            glClearColor(Framebuffer->ClearColor.X, Framebuffer->ClearColor.Y, Framebuffer->ClearColor.Z, Framebuffer->ClearColor.W);
            glClear(GL_COLOR_BUFFER_BIT);

            Editor->EditorDraw(SVec2(ScaledSize));

            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            Editor->EditorUpdate();

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2());
            ImTextureID TexID = Framebuffer->ColorID;
            ImGui::ImageButton("Tex", TexID, ImVec2((float)ScaledSize.X * CanvasScale, (float)ScaledSize.Y * CanvasScale),
                ImVec2(0.0f, (float)ScaledSize.Y / (float)Framebuffer->Height), ImVec2((float)ScaledSize.X / (float)Framebuffer->Width, 0.0f));
            ImGui::PopStyleVar();

            if (ImGui::IsWindowHovered())
            {
                Editor->Scale += IO.MouseWheel * 1.0f;
                Editor->Scale = std::floor(Editor->Scale);
                Editor->Scale = std::max(1.0f, Editor->Scale);
                if (ImGui::IsMouseDragging(2))
                {
                    ImVec2 DragDelta = ImGui::GetMouseDragDelta(2);
                    ImGui::ResetMouseDragDelta(2);

                    Editor->CursorPosition -= SVec4(SVec2{ DragDelta.x, DragDelta.y } / CanvasScale);
                    Log::DevTools<ELogLevel::Verbose>("Drag registered: X = %.2f, Y = %.2f", DragDelta.x, DragDelta.y);
                }
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();
}
//
// static void FitEditorWindow(const SVec2& BaseSize, float* Scale)
// {
//     auto MapSizePixels = Level.CalculateMapSize();
//
//     auto Viewport = ImGui::GetMainViewport();
//     Scale = std::min((float)Viewport->Size.x / (float)MapSizePixels.X, (float)Viewport->Size.y / (float)MapSizePixels.Y);
//     Scale = std::max(1.0f, (std::floor(Scale) - 1.0f));
//     bResetGridPosition = true;
// }

void SWorldEditor::Init(SGame* InGame)
{
    Game = InGame;
    Framebuffer.Init(2048, 2048, { 0.0f, 1.0f, 0.0f, 1.0f });
}

void SWorldEditor::Cleanup()
{
    Game = nullptr;
    Framebuffer.Cleanup();
}

void SWorldEditor::EditorDraw(const SVec2& ScaledSize)
{
    auto& Renderer = Game->Renderer;
    Renderer.GlobalsUniformBlock.SetVector2(offsetof(SShaderGlobals, ScreenSize), ScaledSize);
    Renderer.ProgramMap.SetCursor(CursorPosition.XY());
    Renderer.DrawWorldMapImmediate({ 0.0f, 0.0f }, ScaledSize);
}

void SWorldEditor::EditorUpdate()
{
}

void SWorldEditor::EditorTools()
{
    static ImGuiID PopupID = ImHashStr("WorldEditorPopup");

    static std::string SavePathString{};

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New", "Ctrl+N"))
            {
                ImGui::PushOverrideID(PopupID);
                ImGui::OpenPopup("New World");
                ImGui::PopID();
            }
            if (ImGui::MenuItem("Load", "Ctrl+L"))
            {
                ImGui::PushOverrideID(PopupID);
                ImGui::OpenPopup("Load World");
                ImGui::PopID();

                // ScanForLevels();
            }
            if (ImGui::MenuItem("Save", "Ctrl+S"))
            {
                ImGui::PushOverrideID(PopupID);
                ImGui::OpenPopup("Save World");
                ImGui::PopID();

                // SavePathString = (std::filesystem::path(EQUINOX_REACH_ASSET_PATH "Map/NewMap" + MapExtension.string())).make_preferred().string();
                // ScanForLevels();
            }
            ImGui::Separator();
            ImGui::MenuItem("Properties", "F5");
            if (ImGui::MenuItem("Validate", "F6"))
            {
                ImGui::PushOverrideID(PopupID);
                ImGui::OpenPopup("Validation Result");
                ImGui::PopID();

                // ValidationResult = Validate(false);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            // ImGui::MenuItem("Show Edges", nullptr, &bDrawEdges);
            // ImGui::MenuItem("Show Wall Joints", nullptr, &bDrawWallJoints);
            // ImGui::MenuItem("Show Grid Lines", nullptr, &bDrawGridLines);
            // ImGui::Separator();
            // if (ImGui::MenuItem("Fit to Screen", "Home"))
            // {
            //     FitTilemapToWindow();
            // }
            ImGui::EndMenu();
        }

        ImGui::Separator();

        // const char* EditorModes[] = { "Normal", "Block Selection", "Toggle Door" };
        // ImGui::Text("Current Mode: %s", EditorModes[(int)LevelEditorMode]);

        ImGui::EndMenuBar();
    }

    // ImGui::PushOverrideID(PopupID);
    // if (ImGui::BeginPopupModal("New Level", nullptr,
    //         ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse))
    // {
    //     ImGui::SliderInt("Width", &NewLevelSize.X, 8, MAX_LEVEL_WIDTH);
    //     ImGui::SliderInt("Height", &NewLevelSize.Y, 8, MAX_LEVEL_HEIGHT);
    //     if (ImGui::Button("Accept"))
    //     {
    //         Level = SWorldLevel{ { NewLevelSize.X, NewLevelSize.Y } };
    //         FitTilemapToWindow();
    //         ImGui::CloseCurrentPopup();
    //     }
    //     ImGui::EndPopup();
    // }
    // ImGui::PopID();
    //
    // const float ModalWidth = ImGui::GetFontSize() * 30;
    // const float ModalHeight = ImGui::GetFontSize() * 15;
    //
    // ImGui::PushOverrideID(PopupID);
    // if (ImGui::BeginPopupModal("Save Level", nullptr,
    //         ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse))
    // {
    //     ImGui::BeginChild("Available Levels", ImVec2(ModalWidth, ModalHeight), ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX);
    //
    //     for (auto& LevelPath : AvailableMaps)
    //     {
    //         if (ImGui::Selectable(LevelPath.string().c_str(), LevelPath.string() == SavePathString))
    //         {
    //             SavePathString = LevelPath.string();
    //         }
    //     }
    //     ImGui::EndChild();
    //
    //     ImGui::PushItemWidth(ModalWidth);
    //     ImGui::InputText("##SaveAs", &SavePathString);
    //     ImGui::PopItemWidth();
    //
    //     ImGui::BeginGroup();
    //     if (ImGui::Button("Accept"))
    //     {
    //         auto SavePath = std::filesystem::path(SavePathString);
    //         if (SavePath.extension() == MapExtension)
    //         {
    //             SaveTilemapToFile(SavePath);
    //             ImGui::CloseCurrentPopup();
    //         }
    //     }
    //     ImGui::SameLine();
    //     if (ImGui::Button("Cancel"))
    //     {
    //         ImGui::CloseCurrentPopup();
    //     }
    //     ImGui::EndGroup();
    //
    //     ImGui::EndPopup();
    // }
    // ImGui::PopID();
    //
    // ImGui::PushOverrideID(PopupID);
    // if (ImGui::BeginPopupModal("Load Level", nullptr,
    //         ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse))
    // {
    //     ImGui::BeginChild("Available Levels", ImVec2(ModalWidth, ModalHeight), ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX);
    //     static std::filesystem::path* LoadPath = nullptr;
    //     for (auto& LevelPath : AvailableMaps)
    //     {
    //         if (ImGui::Selectable(LevelPath.string().c_str(), &LevelPath == LoadPath))
    //         {
    //             LoadPath = &LevelPath;
    //         }
    //     }
    //     ImGui::EndChild();
    //
    //     ImGui::BeginGroup();
    //     if (ImGui::Button("Accept"))
    //     {
    //         if (LoadPath != nullptr)
    //         {
    //             LoadTilemapFromFile(*LoadPath);
    //             ImGui::CloseCurrentPopup();
    //         }
    //     }
    //     ImGui::SameLine();
    //     if (ImGui::Button("Cancel"))
    //     {
    //         ImGui::CloseCurrentPopup();
    //     }
    //     ImGui::EndGroup();
    //
    //     ImGui::EndPopup();
    // }
    // ImGui::PopID();
    //
    // ImGui::PushOverrideID(PopupID);
    // if (ImGui::BeginPopupModal("Validation Result", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    // {
    //     ImGui::Text("Found Issues:");
    //     ImGui::Text("Wall: %d", ValidationResult.Wall);
    //     ImGui::Text("Door: %d", ValidationResult.Door);
    //     ImGui::Separator();
    //
    //     if (ImGui::Button("OK", ImVec2(120, 0)))
    //     {
    //         Validate(true);
    //         ImGui::CloseCurrentPopup();
    //     }
    //     ImGui::SetItemDefaultFocus();
    //     ImGui::SameLine();
    //     if (ImGui::Button("Cancel", ImVec2(120, 0)))
    //     {
    //         ImGui::CloseCurrentPopup();
    //     }
    //     ImGui::EndPopup();
    // }
    // ImGui::PopID();

    static int SelectedIndex = 0;
    // ImGui::BeginGroup();
    ImGui::BeginChild("WorldEditorTools", ImVec2(ImGui::GetFontSize() * 14.0f, 0.0f)); // Leave room for 1 line below us
    // ImGui::Text("MyObject: %d", selected);
    // ImGui::Separator();
    if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
    {
        if (ImGui::BeginTabItem("Layers"))
        {
            ImGui::BeginChild("LayersList", ImVec2(0, 0), ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);
            const ImVec2 EntrySize{ 0, ImGui::GetFontSize() * 4.0f };
            for (int Index = 0; Index < (int)World.Levels.size(); Index++)
            {
                auto& WorldLevel = World.Levels[Index];
                const bool bSelected = Index == SelectedIndex;
                auto CursorPos = ImGui::GetCursorPos();

                char Label[128];
                sprintf(Label, "##LayerEntry%d", Index);

                const ImU32 Color = ImGui::GetColorU32(bSelected ? ImGuiCol_HeaderActive
                                                                 : ImGuiCol_Header);
                ImGui::PushStyleColor(ImGuiCol_FrameBg, Color);
                ImGui::BeginChild(Label, EntrySize, ImGuiChildFlags_Border | ImGuiChildFlags_FrameStyle);
                ImGui::ColorEdit4("##LayerColor", &WorldLevel.Color.X, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoInputs);
                ImGui::SameLine();
                ImGui::Text("#%d", Index);
                // ImGui::Text(WorldLevel.);
                if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0))
                {
                    SelectedIndex = Index;
                }
                ImGui::EndChild();
                ImGui::PopStyleColor();
                // if (ImGui::Selectable(Label, selected == i))
                // {
                //     selected = i;
                // }
                // ImGui::SetItemAllowOverlap();
                // ImGui::SetCursorPos(CursorPos);
                // if (ImGui::Button("do thing", ImVec2(70, 30)))
                // {
                //     ImGui::OpenPopup("Setup?");
                //     // selected = n;
                //     printf("SETUP CLICKED %d\n", 1);
                // }
            }
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Properties"))
        {
            ImGui::Text("ID: 0123456789");
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::EndChild();
    // if (ImGui::Button("Revert"))
    // {
    // }
    // ImGui::SameLine();
    // if (ImGui::Button("Save"))
    // {
    // }
    // ImGui::EndGroup();
}

void SWorldEditor::RenderLayers(SGame& Game)
{
}

void SLevelEditor::Init(SGame* InGame)
{
    Game = InGame;
    LevelEditorMode = ELevelEditorMode::Normal;
    NewLevelSize = SVec2Int{ 8, 8 };
    Scale = 1.0f;
    bDrawWallJoints = false;
    bDrawEdges = true;
    bDrawGridLines = false;
    Level = SWorldLevel{ { 8, 8 } };
    Framebuffer.Init(2048, 2048, { 0.0f, 0.0f, 1.0f, 1.0f });
}

void SLevelEditor::Cleanup()
{
    Game = nullptr;
    Framebuffer.Cleanup();
}

void SLevelEditor::EditorDraw(const SVec2& ScaledSize)
{
    auto& Renderer = Game->Renderer;
    Renderer.GlobalsUniformBlock.SetVector2(offsetof(SShaderGlobals, ScreenSize), ScaledSize);
    Renderer.ProgramMap.SetCursor(CursorPosition.XY());
    if (bLevelChanged)
    {
        Renderer.UploadMapData(&Level, Game->Blob.UnreliableCoordsAndDirection());
        bLevelChanged = false;
    }
    if (bEditorStateChanged)
    {
        Renderer.ProgramMap.SetEditorData(
            SVec2(SelectedTileCoords),
            SVec4(BlockModeTileCoords, SelectedTileCoords),
            true,
            LevelEditorMode == ELevelEditorMode::ToggleEdge,
            LevelEditorMode == ELevelEditorMode::Block);
        bEditorStateChanged = false;
    }
    Renderer.DrawMapImmediate({ 0.0f, 0.0f }, ScaledSize);
}

void SLevelEditor::EditorUpdate()
{
    ImVec2 WindowSize = ImGui::GetWindowSize();
    ImVec2 CursorPos = ImGui::GetCursorScreenPos();

    SVec2Int OriginalMapSize = Level.CalculateMapSize();

    if (ImGui::IsWindowHovered())
    {
        if (ImGui::IsMouseClicked(0))
        {
            auto AbsoluteMousePos = ImGui::GetMousePos();
            SVec2 MousePos{ AbsoluteMousePos.x - CursorPos.x, AbsoluteMousePos.y - CursorPos.y };
            MousePos /= Scale;
            MousePos += CursorPosition.XY();
            MousePos -= SVec2{ WindowSize.x * 0.5f, WindowSize.y * 0.5f } / Scale;
            MousePos += SVec2(OriginalMapSize) * 0.5f;
            SVec2Int NewSelectedTileCoords{
                (int)std::floor(MousePos.X / MAP_TILE_SIZE_PIXELS),
                (int)std::floor(MousePos.Y / MAP_TILE_SIZE_PIXELS)
            };
            if (Level.IsValidTile(NewSelectedTileCoords))
            {
                SelectedTileCoords = NewSelectedTileCoords;
                bEditorStateChanged = true;
            }
        }
    }

    if (ImGui::IsWindowFocused())
    {
        if (ImGui::IsKeyPressed(ImGuiKey_Escape))
        {
            LevelEditorMode = ELevelEditorMode::Normal;
        }
        else if (LevelEditorMode == ELevelEditorMode::Normal || LevelEditorMode == ELevelEditorMode::Block)
        {
            if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))
            {
                SelectedTileCoords.Y = std::max(0, SelectedTileCoords.Y - 1);
                bEditorStateChanged = true;
            }
            if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))
            {
                SelectedTileCoords.X = std::max(0, SelectedTileCoords.X - 1);
                bEditorStateChanged = true;
            }
            if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
            {
                SelectedTileCoords.Y = std::min(Level.Height - 1, SelectedTileCoords.Y + 1);
                bEditorStateChanged = true;
            }
            if (ImGui::IsKeyPressed(ImGuiKey_RightArrow))
            {
                SelectedTileCoords.X = std::min(Level.Width - 1, SelectedTileCoords.X + 1);
                bEditorStateChanged = true;
            }
        }
        if (LevelEditorMode == ELevelEditorMode::Block)
        {
            if (ImGui::IsKeyPressed(ImGuiKey_Space))
            {
                Level.EditBlock(SRectInt::FromTwo(SelectedTileCoords, BlockModeTileCoords), TILE_FLOOR_BIT);
                LevelEditorMode = ELevelEditorMode::Normal;
                bEditorStateChanged = true;
                bLevelChanged = true;
            }
            if (ImGui::IsKeyPressed(ImGuiKey_C))
            {
                Level.EditBlock(SRectInt::FromTwo(SelectedTileCoords, BlockModeTileCoords), 0);
                LevelEditorMode = ELevelEditorMode::Normal;
                bEditorStateChanged = true;
                bLevelChanged = true;
            }
            if (ImGui::IsKeyPressed(ImGuiKey_H))
            {
                Level.EditBlock(SRectInt::FromTwo(SelectedTileCoords, BlockModeTileCoords), TILE_HOLE_BIT);
                LevelEditorMode = ELevelEditorMode::Normal;
                bEditorStateChanged = true;
                bLevelChanged = true;
            }
        }
        else if (LevelEditorMode == ELevelEditorMode::Normal)
        {
            if (ImGui::IsKeyPressed(ImGuiKey_Space))
            {
                Level.Edit(SelectedTileCoords, TILE_FLOOR_BIT);
                bLevelChanged = true;
            }
            if (ImGui::IsKeyPressed(ImGuiKey_C))
            {
                Level.Edit(SelectedTileCoords, 0);
                bLevelChanged = true;
            }
            if (ImGui::IsKeyPressed(ImGuiKey_H))
            {
                Level.Edit(SelectedTileCoords, TILE_HOLE_BIT);
                bLevelChanged = true;
            }
            if (ImGui::IsKeyPressed(ImGuiKey_D))
            {
                LevelEditorMode = ELevelEditorMode::ToggleEdge;
                ToggleEdgeType = TILE_EDGE_DOOR_BIT;
                bEditorStateChanged = true;
            }
            if (ImGui::IsKeyPressed(ImGuiKey_W))
            {
                LevelEditorMode = ELevelEditorMode::ToggleEdge;
                ToggleEdgeType = TILE_EDGE_WALL_BIT;
                bEditorStateChanged = true;
            }
            if (ImGui::IsKeyPressed(ImGuiKey_V))
            {
                LevelEditorMode = ELevelEditorMode::Block;
                BlockModeTileCoords = SelectedTileCoords;
                bEditorStateChanged = true;
            }
        }
        else if (LevelEditorMode == ELevelEditorMode::ToggleEdge)
        {
            auto ToggleEdge = [&, this](SDirection Direction) {
                Level.ToggleEdge(SelectedTileCoords, Direction, ToggleEdgeType);
                LevelEditorMode = ELevelEditorMode::Normal;
                bEditorStateChanged = true;
                bLevelChanged = true;
            };
            if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))
            {
                ToggleEdge(SDirection::North());
            }
            else if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))
            {
                ToggleEdge(SDirection::West());
            }
            else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
            {
                ToggleEdge(SDirection::South());
            }
            else if (ImGui::IsKeyPressed(ImGuiKey_RightArrow))
            {
                ToggleEdge(SDirection::East());
            }
        }
    }

    if (ImGui::IsKeyPressed(ImGuiKey_Home) || ImGui::IsKeyPressed(ImGuiKey_Keypad7) || bResetView)
    {
        Scale = std::min(WindowSize.x / (float)OriginalMapSize.X, WindowSize.y / (float)OriginalMapSize.Y);
        Scale = std::max(1.0f, (std::floor(Scale) - 1.0f));
        Log::DevTools<ELogLevel::Critical>("%f", Scale);
        CursorPosition.X = 0.0f;
        CursorPosition.Y = 0.0f;
        bEditorStateChanged = true;
        bResetView = false;
    }
}

void SLevelEditor::EditorTools()
{
    static ImGuiID PopupID = ImHashStr("LevelEditorPopup");

    static SValidationResult ValidationResult;
    static std::string SavePathString{};

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New", "Ctrl+N"))
            {
                ImGui::PushOverrideID(PopupID);
                ImGui::OpenPopup("New Level");
                ImGui::PopID();
            }
            if (ImGui::MenuItem("Load", "Ctrl+L"))
            {
                ImGui::PushOverrideID(PopupID);
                ImGui::OpenPopup("Load Level");
                ImGui::PopID();

                ScanForLevels();
            }
            if (ImGui::MenuItem("Save", "Ctrl+S"))
            {
                ImGui::PushOverrideID(PopupID);
                ImGui::OpenPopup("Save Level");
                ImGui::PopID();

                SavePathString = (std::filesystem::path(EQUINOX_REACH_ASSET_PATH "Map/NewMap" + MapExtension.string())).make_preferred().string();
                ScanForLevels();
            }
            ImGui::Separator();
            ImGui::MenuItem("Properties", "F5");
            if (ImGui::MenuItem("Validate", "F6"))
            {
                ImGui::PushOverrideID(PopupID);
                ImGui::OpenPopup("Validation Result");
                ImGui::PopID();

                ValidationResult = Validate(false);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            ImGui::MenuItem("Show Edges", nullptr, &bDrawEdges);
            ImGui::MenuItem("Show Wall Joints", nullptr, &bDrawWallJoints);
            ImGui::MenuItem("Show Grid Lines", nullptr, &bDrawGridLines);
            ImGui::Separator();
            if (ImGui::MenuItem("Fit to Screen", "Home"))
            {
                bResetView = true;
            }
            ImGui::EndMenu();
        }

        ImGui::Separator();

        const char* EditorModes[] = { "Normal", "Block Selection", "Toggle Door" };
        ImGui::Text("Current Mode: %s", EditorModes[(int)LevelEditorMode]);

        ImGui::Separator();
        ImGui::Text("Scale: %.1fx", Scale);

        ImGui::EndMenuBar();
    }

    ImGui::PushOverrideID(PopupID);
    if (ImGui::BeginPopupModal("New Level", nullptr,
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse))
    {
        ImGui::SliderInt("Width", &NewLevelSize.X, 8, MAX_LEVEL_WIDTH);
        ImGui::SliderInt("Height", &NewLevelSize.Y, 8, MAX_LEVEL_HEIGHT);
        if (ImGui::Button("Accept"))
        {
            Level = SWorldLevel{ { NewLevelSize.X, NewLevelSize.Y } };
            bLevelChanged = true;
            bResetView = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    ImGui::PopID();

    const float ModalWidth = ImGui::GetFontSize() * 30;
    const float ModalHeight = ImGui::GetFontSize() * 15;

    ImGui::PushOverrideID(PopupID);
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
    ImGui::PopID();

    ImGui::PushOverrideID(PopupID);
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
    ImGui::PopID();

    ImGui::PushOverrideID(PopupID);
    if (ImGui::BeginPopupModal("Validation Result", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Found Issues:");
        ImGui::Text("Wall: %d", ValidationResult.Wall);
        ImGui::Text("Door: %d", ValidationResult.Door);
        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            Validate(true);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    ImGui::PopID();

    /* Validate selected tiles. */
    if (!Level.IsValidTile(SelectedTileCoords))
    {
        SelectedTileCoords = { 0, 0 };
    }
    if (!Level.IsValidTile(BlockModeTileCoords))
    {
        BlockModeTileCoords = SelectedTileCoords;
    }

    auto SelectedTile = Level.GetTileAtMutable(SelectedTileCoords);

    if (ImGui::BeginChild("Tile Settings", ImVec2(ImGui::GetFontSize() * 14, 0),
            ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_Border))
    {
        ImGui::Text("X = %d, Y = %d", SelectedTileCoords.X, SelectedTileCoords.Y);
        ImGui::Text("Flags = %d", SelectedTile->Flags);
        ImGui::Text("Special Flags = %d", SelectedTile->SpecialFlags);
        ImGui::Text("Edge Flags = %d", SelectedTile->EdgeFlags);
        ImGui::Text("Special Edge Flags = %d", SelectedTile->SpecialEdgeFlags);

        ImGui::SetNextItemWidth(ImGui::GetFontSize() * 100);
        ImGui::Separator();
        // const char* TileTypes[] = { "Empty", "Floor", "Hole" };
        // const char* EdgeTypes[] = { "Empty", "Wall", "Door" };

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
    }
    ImGui::EndChild();
}

void SLevelEditor::SaveTilemapToFile(const class std::filesystem::path& Path)
{
    Validate(true);
    bLevelChanged = true;

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
    bLevelChanged = true;
    bResetView = true;
}

void SLevelEditor::ScanForLevels()
{
    namespace fs = std::filesystem;
    AvailableMaps.clear();
    auto MapsFolder = fs::path(EQUINOX_REACH_ASSET_PATH "Map").make_preferred();
    for (const auto& File : fs::recursive_directory_iterator(MapsFolder))
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

SValidationResult SLevelEditor::Validate(bool bFix)
{
    static SWorldLevel TempLevel;
    SWorldLevel* TargetLevel;
    if (bFix)
    {
        TargetLevel = &Level;
    }
    else
    {
        TempLevel = Level;
        TargetLevel = &TempLevel;
    }
    SValidationResult Result;
    SVec2Int Coords{};

    auto ValidateEdge = [&](STile* CurrentTile, STile* NeighborTile, SDirection Direction, SDirection NeighborDirection, UFlagType EdgeBit, int* Corrections) {
        if (CurrentTile->CheckEdgeFlag(EdgeBit, Direction))
        {
            UFlagType Temp = NeighborTile->EdgeFlags;
            NeighborTile->ClearEdgeFlags(NeighborDirection);
            NeighborTile->SetEdgeFlag(EdgeBit, NeighborDirection);
            if (Temp != NeighborTile->EdgeFlags)
            {
                *Corrections = *Corrections + 1;
            }
        }
    };

    for (; Coords.X < TargetLevel->Width; ++Coords.X)
    {
        for (Coords.Y = 0; Coords.Y < TargetLevel->Height; ++Coords.Y)
        {
            auto CurrentTile = TargetLevel->GetTileAtMutable(Coords);

            /* @TODO: Should validate these? */
            CurrentTile->ClearSpecialFlag(TILE_SPECIAL_VISITED_BIT);
            CurrentTile->ClearSpecialFlag(TILE_SPECIAL_EXPLORED_BIT);

            for (auto& Direction : SDirection::All())
            {
                auto NeighborTile = TargetLevel->GetTileAtMutable(Coords + Direction.GetVector<int>());
                if (NeighborTile != nullptr)
                {
                    auto NeighborDirection = Direction.Inverted();
                    ValidateEdge(CurrentTile, NeighborTile, Direction, NeighborDirection, TILE_EDGE_WALL_BIT, &Result.Wall);
                    ValidateEdge(CurrentTile, NeighborTile, Direction, NeighborDirection, TILE_EDGE_DOOR_BIT, &Result.Door);
                }
            }
        }
    }

    Log::DevTools<ELogLevel::Critical>("[Validate] Successful!");
    Log::DevTools<ELogLevel::Critical>("[Validate] Corrections: Walls = %d, Doors = %d", Result.Wall, Result.Door);

    return Result;
}

void SDevTools::DrawParty(SParty& Party, float Scale, [[maybe_unused]] bool bReversed)
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

void SDevTools::Init(SGame* InGame)
{
    Game = InGame;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL3_InitForOpenGL(Game->Platform.Window, Game->Platform.Context);
    ImGui_ImplOpenGL3_Init(Constants::GLSLVersion);

    LevelEditor.Init(InGame);
    WorldEditor.Init(InGame);

    float WindowScale = SDL_GetWindowDisplayScale(Game->Platform.Window);
    ImGui::GetStyle().ScaleAllSizes(WindowScale);

    /* Don't transfer asset ownership to ImGui, it will crash otherwise! */
    ImFontConfig FontConfig;
    FontConfig.FontDataOwnedByAtlas = false;
    io.Fonts->AddFontFromMemoryTTF(Asset::Common::MartianMono.VoidPtr(), (int)Asset::Common::MartianMono.Length, std::floor(16.0f * WindowScale), &FontConfig);
}

void SDevTools::Cleanup()
{
    LevelEditor.Cleanup();
    WorldEditor.Cleanup();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    Game = nullptr;
}

void SDevTools::Update()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    if (ImGui::IsKeyPressed(ImGuiKey_F4))
    {
        // Game.Renderer.ProgramHUD.Reload();
        Game->Renderer.ProgramMap.Reload();
        // Game.Renderer.ProgramUber2D.Reload();
        // Game.Renderer.ProgramUber3D.Reload();
    }

    bool bEditorOpened{};
    bool bReturnedToGame{};
    EDevToolsMode OldMode = Mode;

    if (ImGui::IsKeyPressed(ImGuiKey_F8))
    {
        Mode = Mode == EDevToolsMode::WorldEditor ? EDevToolsMode::Game : EDevToolsMode::WorldEditor;
    }

    if (ImGui::IsKeyPressed(ImGuiKey_F9))
    {
        Mode = Mode == EDevToolsMode::LevelEditor ? EDevToolsMode::Game : EDevToolsMode::LevelEditor;
    }

    if (Mode != OldMode)
    {
        if (Mode == EDevToolsMode::Game)
        {
            bReturnedToGame = true;
        }
        else
        {
            bEditorOpened = true;
        }
    }

    if (bEditorOpened)
    {
        Game->Renderer.ProgramMap.SetEditorData(SVec2(), SVec4(), true, false, false);
    }

    if (bReturnedToGame)
    {
        /* @TODO: Revert cursor changes? */
        Game->Renderer.ProgramMap.SetEditorData(SVec2(), SVec4(), false, false, false);
        Game->Renderer.UploadMapData(Game->World.GetLevel(), Game->Blob.UnreliableCoordsAndDirection());
    }

    switch (Mode)
    {
        case EDevToolsMode::LevelEditor:
            GenericEditorWindow(
                "Level Editor", &LevelEditor);
            break;
        case EDevToolsMode::WorldEditor:
            GenericEditorWindow(
                "World Editor", &WorldEditor);
            break;
        default:
            ShowDebugTools();
            break;
    }
}

void SDevTools::ShowDebugTools() const
{
    if (ImGui::Begin("Debug Tools", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        if (ImGui::TreeNode("System Info"))
        {
            ImGui::Text("Frames Per Second: %.f", 1000.0f / Game->Platform.DeltaTime / 1000.0f);
            ImGui::Text("Number Of Blocks: %zu", Memory::NumberOfBlocks());
            ImGui::Text("Display Scale: %d", Game->Renderer.MainFramebuffer.Scale);
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Player Info"))
        {
            ImGui::Text("Direction: %s (%u)", SDirection::Names[Game->Blob.Direction.Index], Game->Blob.Direction.Index);
            ImGui::Text("Coords: X=%d, Y=%d", Game->Blob.Coords.X, Game->Blob.Coords.Y);
            ImGui::Text("Unreliable Coords: X=%.2f, Y=%.2f", Game->Blob.UnreliableCoords().X, Game->Blob.UnreliableCoords().Y);
            DrawParty(Game->PlayerParty, ImGui::GetFontSize() * 0.01f, true);
            DrawParty(Game->PlayerParty, ImGui::GetFontSize() * 0.01f, false);
            ImGui::TreePop();
        }
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode("Level Tools"))
        {
            SWorldLevel* Level = Game->World.GetLevel();
            if (ImGui::Button("Explore Level"))
            {
                for (auto X = 0; X < Level->Width; X++)
                {
                    for (auto Y = 0; Y < Level->Height; Y++)
                    {
                        auto Tile = Level->GetTileAtMutable({ X, Y });
                        if (Tile != nullptr)
                        {
                            Tile->SetSpecialFlag(TILE_SPECIAL_EXPLORED_BIT);
                        }
                    }
                }
                Level->DirtyFlags = ELevelDirtyFlags::All;
                Level->DirtyRange = { 0, Level->TileCount() };
            }
            if (ImGui::Button("Visit Level"))
            {
                for (auto X = 0; X < Level->Width; X++)
                {
                    for (auto Y = 0; Y < Level->Height; Y++)
                    {
                        auto Tile = Level->GetTileAtMutable({ X, Y });
                        if (Tile != nullptr)
                        {
                            Tile->SetSpecialFlag(TILE_SPECIAL_EXPLORED_BIT);
                            Tile->SetSpecialFlag(TILE_SPECIAL_VISITED_BIT);
                        }
                    }
                }
                Level->DirtyFlags = ELevelDirtyFlags::All;
                Level->DirtyRange = { 0, Level->TileCount() };
            }
            if (ImGui::Button("Import Level From Editor"))
            {
                Game->ChangeLevel(LevelEditor.Level);
            }
            ImGui::TreePop();
        }
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode("Adjustments"))
        {
            ImGui::SliderFloat("##TimeScale", &Game->Platform.TimeScale, 0.0f, 4.0f, "Time Scale: %.2f");
            ImGui::SliderFloat("##MasterVolume", &Game->Audio.Volume, 0.0f, 1.0f, "Master Volume: %.2f");
            ImGui::SliderFloat("##InputBufferTime", &Game->Blob.InputBufferTime, 0.0f, 1.0f, "Input Buffer Time: %.3f");
            ImGui::TreePop();
        }
    }
    ImGui::End();
}

void SDevTools::Draw() const
{
    ImGui::Render();
    ImGuiIO& io = ImGui::GetIO();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    if (Mode != EDevToolsMode::Game)
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
