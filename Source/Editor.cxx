#include "Editor.hxx"

#include <iostream>
#include "glad/gl.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl2.h"
#include "Constants.hxx"

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
                ImGui::MenuItem("Level Properties", nullptr, &bLevelProperties);
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        if (bNewLevel) {
            ImGui::OpenPopup("New Level...");
        }
        if (ImGui::BeginPopupModal("New Level...", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse)) {
            ImGui::SliderInt("Width", &NewLevelWidth, 8, 64);
            ImGui::SliderInt("Height", &NewLevelHeight, 8, 64);
            if (ImGui::Button("Accept")) {
                Level = std::make_shared<SLevel>();
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        if (IsLevelLoaded()) {
            if (ImGui::Begin("Grid")) {
                ImGui::Text("Grid");
                ImDrawList* DrawList = ImGui::GetWindowDrawList();
                DrawList->AddRectFilled(ImVec2(0, 0), ImVec2(22, 22), 0);
                ImGui::GetForegroundDrawList()->AddRect(ImVec2(0,0), ImVec2(22,22), 0);
                ImGui::End();
            }
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