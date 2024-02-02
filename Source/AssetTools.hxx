#pragma once

#include <string>
#include <memory_resource>
#include "Math.hxx"

#ifdef EQUINOX_REACH_DEVELOPMENT
    #include <filesystem>
#endif

#define EXTERN_ASSET(NAME) extern const SAsset NAME;

struct SAsset
{
    const unsigned char* Data;
    size_t Length;
#ifdef EQUINOX_REACH_DEVELOPMENT
    const char* BasePath;
    const char* RelativeAssetPath;

    explicit SAsset(const char* InData, size_t InLength, const char* InBasePath, const char* InRelativeAssetPath);

    [[nodiscard]] std::filesystem::path Path() const;
#else
    explicit SAsset(const char* InData, size_t InLength);
#endif

    [[nodiscard]] std::string String() const { return { SignedCharPtr(), Length }; }

    [[nodiscard]] const char* SignedCharPtr() const
    {
        return reinterpret_cast<const char*>(Data);
    }

    [[nodiscard]] void* VoidPtr() const
    {
        return reinterpret_cast<void*>(const_cast<unsigned char*>(Data));
    }
};

class CRawMesh
{
public:
    std::pmr::vector<UVec3> Positions;
    std::pmr::vector<UVec2> TexCoords;
    std::pmr::vector<UVec3> Normals;
    std::pmr::vector<unsigned short> Indices;

    [[nodiscard]] int GetVertexCount() const { return (int)Positions.size(); }

    [[nodiscard]] int GetElementCount() const { return (int)Indices.size(); }

    CRawMesh(const SAsset& Resource);
};

class CRawImage
{
public:
    int Width{};
    int Height{};
    int Channels{};
    void* Data{};

    explicit CRawImage(const SAsset& Resource);
    ~CRawImage();
};

class CRawImageInfo
{
public:
    int Width{};
    int Height{};
    int Channels{};

    explicit CRawImageInfo(const SAsset& Resource);
};
