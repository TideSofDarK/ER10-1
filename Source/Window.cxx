#include "Window.hxx"

#include <iostream>
#include "Constants.hxx"
#include <glad/gl.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include "Memory.hxx"

void SWindow::SwapBuffers() const
{
    SDL_GL_SwapWindow(Window);
}

bool SWindow::IsAnyFullscreen() const
{
    return (SDL_GetWindowFlags(Window) & SDL_WINDOW_FULLSCREEN) != 0;
}

void SWindow::Init()
{
    if (Window != nullptr)
    {
        exit(1);
    }

    if (SDL_SetMemoryFunctions(&CMemory::Malloc, &CMemory::Calloc, &CMemory::Realloc, &CMemory::Free))
    {
        SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
        exit(1);
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
        exit(1);
    }

    Window = SDL_CreateWindow(GAME_NAME,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    SDL_SetWindowPosition(Window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_GLContext gl_context = SDL_GL_CreateContext(Window);
    SDL_GL_MakeCurrent(Window, gl_context);
    SDL_GL_SetSwapInterval(1);
    SDL_ShowWindow(Window);

    gladLoadGL(reinterpret_cast<GLADloadfunc>(SDL_GL_GetProcAddress));

    if (!Window)
    {
        SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
        exit(1);
    }

    SDL_GetWindowSizeInPixels(Window, &Width, &Height);
    OnWindowResized(Width, Height);
}

void SWindow::OnWindowResized(int InWidth, int InHeight)
{
    Width = InWidth;
    Height = InHeight;
    BlitWidth = Width;
    BlitHeight = static_cast<int>(static_cast<float>(BlitWidth) / SCREEN_ASPECT);
    if (Height < BlitHeight)
    {
        BlitHeight = Height;
        BlitWidth = static_cast<int>(static_cast<float>(BlitHeight) * SCREEN_ASPECT);
    }
    BlitX = std::max((Width - BlitWidth) / 2, 0);
    BlitY = std::max((Height - BlitHeight) / 2, 0);
}

void SWindow::Cleanup() const
{
    SDL_GL_DeleteContext(Context);
    SDL_DestroyWindow(Window);
    SDL_Quit();
}

void SWindow::ToggleBorderlessFullscreen() const
{
    SDL_SetWindowFullscreen(Window, !IsAnyFullscreen());
}
