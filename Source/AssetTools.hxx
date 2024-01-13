#pragma once

#include "Memory.hxx"
#include "Math.hxx"

#define EXTERN_ASSET(NAME) extern const SAsset NAME;

struct SAsset
{
    const unsigned char* Data;
    size_t Length;

    explicit SAsset(const char* InData, size_t InLength)
        : Data(reinterpret_cast<const unsigned char*>(InData)), Length(InLength){

        };

    [[nodiscard]] std::string ToString() const { return { AsSignedCharPtr(), Length }; }

    const char* AsSignedCharPtr() const
    {
        return reinterpret_cast<const char*>(Data);
    }

    void* AsVoidPtr() const
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
