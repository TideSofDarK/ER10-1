#pragma once

#include <cstdio>
#include <iostream>

namespace Log
{
    template <typename... Ps>
    static inline void Memory(const char* fmt, Ps... ps)
    {
#ifdef EQUINOX_REACH_DEVELOPMENT
        char Entry[1024];
        char Msg[960];
        snprintf(Msg, 960, fmt, ps...);
        snprintf(Entry, 1024, "[Memory] %s", Msg);
        std::cout << Entry << std::endl;
#endif
    }
}