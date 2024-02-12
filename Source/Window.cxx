#include "Window.hxx"

#include <glad/gl.h>
#include <SDL3/SDL.h>
#include "Log.hxx"
#include "Memory.hxx"
#include "SDL_video.h"
#include "Constants.hxx"

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
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0)
    {
        SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
        exit(1);
    }

#ifdef EQUINOX_REACH_GAME
    SDL_SetEventEnabled(SDL_EVENT_TEXT_INPUT, SDL_FALSE);
#endif

    int DisplayCount{};
    SDL_DisplayID* Displays = SDL_GetDisplays(&DisplayCount);
    SVec2Int Resolution = CalculateOptimalWindowedResolution(Displays[0]);

    Window = SDL_CreateWindow(GAME_NAME,
        Resolution.X, Resolution.Y,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

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

    OnWindowResized();
}

SWindow::~SWindow()
{
    SDL_GL_DeleteContext(Context);
    SDL_DestroyWindow(Window);
    SDL_Quit();
}

void SWindow::OnWindowResized()
{
    SDL_GetWindowSize(Window, &Width, &Height);
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

SVec2Int SWindow::CalculateOptimalWindowedResolution(unsigned DisplayID)
{
    SDL_Rect UsableBounds;

    SDL_GetDisplayUsableBounds(DisplayID, &UsableBounds);

    int ScaleFactorY = UsableBounds.h / SCREEN_HEIGHT;
    int ScaleFactorX = UsableBounds.w / SCREEN_WIDTH;

    int ScaleFactor = std::max(1, (std::min(ScaleFactorX, ScaleFactorY)));

    return { SCREEN_WIDTH * ScaleFactor, SCREEN_HEIGHT * ScaleFactor };
}

void SWindow::SetOptimalWindowedResolution() const
{
    SDL_DisplayID DisplayID = SDL_GetDisplayForWindow(Window);

    auto Resolution = CalculateOptimalWindowedResolution(DisplayID);

    SDL_SetWindowSize(Window, Resolution.X, Resolution.Y);
    SDL_SetWindowPosition(Window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_SetWindowFullscreen(Window, false);
}

namespace Platform
{
    void Init()
    {
        SDL_LogSetAllPriority(SDL_LogPriority::SDL_LOG_PRIORITY_INFO);
        SDL_LogSetOutputFunction([]([[maybe_unused]] void* Userdata, [[maybe_unused]] int Category, [[maybe_unused]] SDL_LogPriority Priority, const char* Message) {
            Log::LogInternal<ELogLevel::Critical>("SDL3", "%s", Message);
        },
            nullptr);

        if (SDL_SetMemoryFunctions(&Memory::Malloc, &Memory::Calloc, &Memory::Realloc, &Memory::Free))
        {
            SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
            exit(1);
        }
    }
}
