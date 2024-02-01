#include "Window.hxx"

#include "Constants.hxx"
#include <glad/gl.h>
#include <SDL3/SDL.h>
#include "Log.hxx"
#include "Memory.hxx"
#include "SDL_oldnames.h"
#include "SDL_video.h"

void SWindow::SwapBuffers() const
{
    SDL_GL_SwapWindow(Window);
}

bool SWindow::IsAnyFullscreen() const
{
    return (SDL_GetWindowFlags(Window) & SDL_WINDOW_FULLSCREEN) != 0;
}

SWindow::SWindow()
{
    SDL_LogSetAllPriority(SDL_LogPriority::SDL_LOG_PRIORITY_INFO);
    SDL_LogSetOutputFunction([](void* Userdata, int Category, SDL_LogPriority Priority, const char* message) {
        Log::LogInternal<ELogLevel::Critical>("SDL3", "%s", message);
    },
        nullptr);

    if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0)
    {
        SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
        exit(1);
    }

#ifdef EQUINOX_REACH_GAME
    SDL_SetEventEnabled(SDL_EVENT_TEXT_INPUT, SDL_FALSE);
#endif

    Window = SDL_CreateWindow(GAME_NAME,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    SetOptimalWindowedResolution();

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
}

SWindow::~SWindow()
{
    SDL_GL_DeleteContext(Context);
    SDL_DestroyWindow(Window);
    SDL_Quit();
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

void SWindow::ToggleBorderlessFullscreen() const
{
    SDL_SetWindowFullscreen(Window, !IsAnyFullscreen());
}

void SWindow::SetOptimalWindowedResolution() const
{
    SDL_Rect UsableBounds;

    SDL_DisplayID DisplayID = SDL_GetDisplayForWindow(Window);

    SDL_GetDisplayUsableBounds(DisplayID, &UsableBounds);

    int ScaleFactorY = UsableBounds.h / SCREEN_HEIGHT;
    int ScaleFactorX = UsableBounds.w / SCREEN_WIDTH;

    int ScaleFactor = std::max(1, (std::min(ScaleFactorX, ScaleFactorY) - 1));

    SDL_SetWindowSize(Window, SCREEN_WIDTH * ScaleFactor, SCREEN_HEIGHT * ScaleFactor);
    SDL_SetWindowPosition(Window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    SDL_Log(
        "DisplayID: %u, ScaleFactor: %d",
        DisplayID,
        ScaleFactor);
}
