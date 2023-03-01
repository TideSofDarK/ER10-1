#pragma once

#ifdef __APPLE__
#include <mach-o/getsect.h>
#define EXTLD(NAME) \
  extern "C" const unsigned char _section$__DATA__ ## NAME [];
#define LDVAR(NAME) _section$__DATA__ ## NAME
#define LDLEN(NAME) (getsectbyname("__DATA", "__" #NAME)->size)
#else
#define EXTLD(NAME) \
  extern "C" const unsigned char _binary_ ## NAME ## _start[]; \
  extern "C" const unsigned char _binary_ ## NAME ## _end[];
#define LDVAR(NAME) \
  _binary_ ## NAME ## _start
#define LDLEN(NAME) \
  ((_binary_ ## NAME ## _end) - (_binary_ ## NAME ## _start))
#endif

typedef const unsigned char *const TResource;
