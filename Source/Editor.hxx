#pragma once

#include <memory>
#include "Level.hxx"

struct SEditor {
    bool bLevelEditorActive;
    int NewLevelWidth;
    int NewLevelHeight;
    std::shared_ptr<SLevel> Level;

    SEditor() {
        bLevelEditorActive = false;
        NewLevelWidth = 8;
        NewLevelHeight = 8;
        Level = nullptr;
    }

    static void Init(struct SDL_Window *Window, void *Context);

    static void Cleanup();

    static void ProcessEvent(const union SDL_Event *Event);

    void Update();

    void Draw() const;

    [[nodiscard]] bool IsLevelLoaded() const {
        return Level != nullptr;
    }
};