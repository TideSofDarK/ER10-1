#pragma once

#include <cstdio>
#include <iostream>

enum class ELogLevel
{
    Critical,
    Info,
    Debug,
    Verbose
};

namespace Log
{
#ifdef EQUINOX_REACH_DEVELOPMENT
    // static constexpr ELogLevel LogLevel = ELogLevel::Verbose;
    static constexpr ELogLevel LogLevel = ELogLevel::Debug;
#else
    static constexpr ELogLevel LogLevel = ELogLevel::Info;
#endif

    template <ELogLevel ThisLogLevel>
    static constexpr void LogInternal(const char* Prefix, const char* Fmt)
    {
        if constexpr (ThisLogLevel <= LogLevel)
        {
            char Entry[1024]{};
            snprintf(Entry, 1024, "[%s] %s", Prefix, Fmt);
            std::cout << Entry << std::endl;
        }
    }

    template <ELogLevel ThisLogLevel, typename... Ps>
    static constexpr void LogInternal(const char* Prefix, const char* Fmt, Ps... Args)
    {
        if constexpr (ThisLogLevel <= LogLevel)
        {
            char Msg[960]{};
            snprintf(Msg, 960, Fmt, Args...);
            LogInternal<ThisLogLevel>(Prefix, Msg);
        }
    }

#define LOG_CATEGORY(Name)                                  \
    template <ELogLevel ThisLogLevel, typename... Ps>       \
    static constexpr void Name(const char* Fmt, Ps... Args) \
    {                                                       \
        LogInternal<ThisLogLevel>(#Name, Fmt, Args...);     \
    }                                                       \
    template <ELogLevel ThisLogLevel>                       \
    static constexpr void Name(const char* Fmt)             \
    {                                                       \
        LogInternal<ThisLogLevel>(#Name, Fmt);              \
    }

    LOG_CATEGORY(Audio)
    LOG_CATEGORY(Memory)
    LOG_CATEGORY(Platform)
    LOG_CATEGORY(Draw)
    LOG_CATEGORY(Game)
#ifdef EQUINOX_REACH_DEVELOPMENT
    LOG_CATEGORY(DevTools)
#endif

#undef LOG_CATEGORY
}
