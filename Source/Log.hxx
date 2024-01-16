#pragma once

#include <cstdio>
#include <iostream>

namespace Log
{
    template <typename... Ps>
    static inline void LogInternal(const char* Prefix, const char* Fmt, Ps... Args)
    {
        char Entry[1024];
        char Msg[960];
        snprintf(Msg, 960, Fmt, Args...);
        snprintf(Entry, 1024, "[%s] %s", Prefix, Msg);
        std::cout << Entry << std::endl;
    }

    template <typename... Ps>
    static inline void Memory(const char* Fmt, Ps... Args)
    {
#ifdef EQUINOX_REACH_DEVELOPMENT
        LogInternal("Memory", Fmt, Args...);
#endif
    }

    template <typename... Ps>
    static inline void Platform(const char* Fmt, Ps... Args)
    {
#ifdef EQUINOX_REACH_DEVELOPMENT
        LogInternal("Platform", Fmt, Args...);
#endif
    }
}
