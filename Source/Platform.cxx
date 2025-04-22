#include "Platform.hxx"

#include <glad/gl.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>
#include "Log.hxx"
#include "Memory.hxx"
#include "Constants.hxx"

void SPlatform::SwapBuffers() const
{
    SDL_GL_SwapWindow(Window);
}

bool SPlatform::IsAnyFullscreen() const
{
    return (SDL_GetWindowFlags(Window) & SDL_WINDOW_FULLSCREEN) != 0;
}

void SPlatform::Init()
{
    SDL_SetLogPriorities(SDL_LogPriority::SDL_LOG_PRIORITY_INFO);
    SDL_SetLogOutputFunction([]([[maybe_unused]] void* Userdata, [[maybe_unused]] int Category, [[maybe_unused]] SDL_LogPriority Priority, const char* Message) {
        Log::LogInternal<ELogLevel::Critical>("SDL3", "%s", Message);
    },
        nullptr);

    if (SDL_SetMemoryFunctions(&Memory::Malloc, &Memory::Calloc, &Memory::Realloc, &Memory::Free) != true)
    {
        SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
        exit(1);
    }

    if (SDL_InitSubSystem(SDL_INIT_VIDEO) != true)
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
    Width = Resolution.X;
    Height = Resolution.Y;

    Window = SDL_CreateWindow(Constants::GameName,
        Width, Height,
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
}

void SPlatform::Cleanup() const
{
    SDL_GL_DestroyContext((SDL_GLContext)Context);
    SDL_DestroyWindow(Window);
    SDL_Quit();
}

void SPlatform::ToggleBorderlessFullscreen() const
{
    SDL_SetWindowFullscreen(Window, !IsAnyFullscreen());
}

SVec2Int SPlatform::CalculateOptimalWindowedResolution(unsigned DisplayID)
{
    SDL_Rect UsableBounds;

    SDL_GetDisplayUsableBounds(DisplayID, &UsableBounds);

    int ScaleFactorX = UsableBounds.w / Constants::ReferenceWidth;
    int ScaleFactorY = UsableBounds.h / Constants::ReferenceHeight;

    int ScaleFactor = std::max(1, (std::min(ScaleFactorX, ScaleFactorY)));

    return { Constants::ReferenceWidth * ScaleFactor, Constants::ReferenceHeight * ScaleFactor };
}

void SPlatform::SetOptimalWindowedResolution() const
{
    SDL_DisplayID DisplayID = SDL_GetDisplayForWindow(Window);

    auto Resolution = CalculateOptimalWindowedResolution(DisplayID);

    SDL_SetWindowSize(Window, Resolution.X, Resolution.Y);
    SDL_SetWindowPosition(Window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_SetWindowFullscreen(Window, false);
}
