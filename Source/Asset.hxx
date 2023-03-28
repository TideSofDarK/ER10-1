#pragma once

#include "glm/vec3.hpp"
#include "glm/vec2.hpp"
#include "Memory.hxx"

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

#define DEFINE_RESOURCE(NAME) \
    EXTLD(NAME) \
    const SResource Resource ## NAME{LDVAR(NAME), LDLEN(NAME)};

struct SResource {
    const unsigned char *const Data;
    ptrdiff_t Length;
};

class CRawMesh {
public:
    std::pmr::vector<glm::vec3> Positions;
    std::pmr::vector<glm::vec2> TexCoords;
    std::pmr::vector<glm::vec3> Normals;
    std::pmr::vector<unsigned short> Indices;

    int GetVertexCount() { return Positions.size(); }

    CRawMesh(const SResource &Resource, CScratchBuffer &ScratchBuffer);
};

class CRawImage {
public:
    int Width{};
    int Height{};
    int Channels{};
    void *Data{};

    CRawImage(const SResource &Resource, CScratchBuffer &ScratchBuffer);
};

class CRawImageInfo {
public:
    int Width{};
    int Height{};
    int Channels{};

    CRawImageInfo(const SResource &Resource, CScratchBuffer &ScratchBuffer);
};
