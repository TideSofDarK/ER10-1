#pragma once

struct SEditor {
    static void Init(struct SDL_Window *Window, void *Context);

    static void Cleanup();

    static void ProcessEvent(const union SDL_Event *Event);

    static void Update();

    static void Draw();
};