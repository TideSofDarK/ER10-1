#pragma once

#include "glm/vec3.hpp"
#include "glm/vec2.hpp"
#include "Memory.hxx"

#define EXTERN_ASSET(NAME) extern const SAsset NAME;

struct SAsset {
    const unsigned char *Data;
    size_t Length;

    [[nodiscard]] std::string ToString() const { return std::string(reinterpret_cast<const char *>(Data), Length); }
};

class CRawMesh {
public:
    std::pmr::vector<glm::vec3> Positions;
    std::pmr::vector<glm::vec2> TexCoords;
    std::pmr::vector<glm::vec3> Normals;
    std::pmr::vector<unsigned short> Indices;

    [[nodiscard]] int GetVertexCount() const { return static_cast<int>(Positions.size()); }

    [[nodiscard]] int GetElementCount() const { return static_cast<int>(Indices.size()); }

    CRawMesh(const SAsset &Resource, CScratchBuffer &ScratchBuffer);
};

class CRawImage {
public:
    int Width{};
    int Height{};
    int Channels{};
    void *Data{};

    CRawImage(const SAsset &Resource, CScratchBuffer &ScratchBuffer);
};

class CRawImageInfo {
public:
    int Width{};
    int Height{};
    int Channels{};

    CRawImageInfo(const SAsset &Resource, CScratchBuffer &ScratchBuffer);
};
