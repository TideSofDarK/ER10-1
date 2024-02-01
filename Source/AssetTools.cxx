#include "AssetTools.hxx"

#include <iostream>
#include <algorithm>
#include <string>
#include <array>
#include <algorithm>
#include "Utility.hxx"
#include "Memory.hxx"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_JPEG
#define STBI_NO_GIF
#define STBI_NO_BMP
#define STBI_NO_PSD
#define STBI_NO_PIC
#define STBI_NO_PNM
#define STBI_NO_HDR
#define STBI_NO_TGA
#define STBI_NO_FAILURE_STRINGS
#define STBI_MALLOC STBIMalloc
#define STBI_REALLOC STBIRealloc
#define STBI_FREE STBIFree

void* STBIMalloc(size_t Length)
{
    return CMemory::Malloc(Length);
}

void* STBIRealloc(void* Pointer, size_t Length)
{
    return CMemory::Realloc(Pointer, Length);
}

void STBIFree(void* Pointer)
{
    CMemory::Free(Pointer);
}

#include <stb/stb_image.h>

/* Keep asset path for development (e.g. reloading) purposes. */
#ifdef EQUINOX_REACH_DEVELOPMENT
SAsset::SAsset(const char* InData, size_t InLength, const char* BasePath, const char* RelativeAssetPath)
    : Data(reinterpret_cast<const unsigned char*>(InData)), Length(InLength), Path(BasePath)
{
    namespace fs = std::filesystem;
    Path = Path.remove_filename();
    Path = Path / fs::path(RelativeAssetPath);
    Path = fs::absolute(Path);
};
#else
SAsset::SAsset(const char* InData, size_t InLength)
    : Data(reinterpret_cast<const unsigned char*>(InData)), Length(InLength){};
#endif

CRawMesh::CRawMesh(const SAsset& Resource)
    : Positions(CMemory::GetVector<UVec3>()), TexCoords(CMemory::GetVector<UVec2>()), Normals(CMemory::GetVector<UVec3>()), Indices(CMemory::GetVector<unsigned short>())
{
    auto ScratchPositions = CMemory::GetVector<UVec3>();
    auto ScratchTexCoords = CMemory::GetVector<UVec2>();
    auto ScratchNormals = CMemory::GetVector<UVec3>();
    auto ScratchOBJIndices = CMemory::GetVector<UVec3Size>();

    Positions.clear();
    Normals.clear();
    TexCoords.clear();

    auto OBJLength = (size_t)Resource.Length;
    std::string_view OBJContents{ reinterpret_cast<const char*>(Resource.Data), OBJLength };

    size_t CurrentIndex{};
    while (CurrentIndex < OBJLength)
    {
        /** Each line is "$token $data" e. g. "v 1.0 1.0 1.0" */
        auto TokenEndIndex = OBJContents.find(' ', CurrentIndex);
        auto Token = std::string_view{ &OBJContents[CurrentIndex], TokenEndIndex - CurrentIndex };
        CurrentIndex = TokenEndIndex;
        auto DataEndIndex = OBJContents.find('\n', CurrentIndex);
        std::string_view Data{ &OBJContents[CurrentIndex + 1], DataEndIndex - CurrentIndex };
        CurrentIndex = DataEndIndex;
        if (Token == "v")
        {
            UVec3 Position{};
            Utility::ParseFloats(Data.data(), Data.data() + Data.size(), &Position.X, 3);
            ScratchPositions.emplace_back(Position);
        }
        else if (Token == "vn")
        {
            UVec3 Normal{};
            Utility::ParseFloats(Data.data(), Data.data() + Data.size(), &Normal.X, 3);
            ScratchNormals.emplace_back(Normal);
        }
        else if (Token == "vt")
        {
            UVec2 TexCoord{};
            Utility::ParseFloats(Data.data(), Data.data() + Data.size(), &TexCoord.X, 2);
            ScratchTexCoords.emplace_back(TexCoord);
        }
        else if (Token == "f")
        {
            std::array<int, 9> OBJIndices{};
            Utility::ParseInts(Data.data(), Data.data() + Data.size(), &OBJIndices[0], OBJIndices.size());
            for (size_t Index = 0; Index < OBJIndices.size(); Index += 3)
            {
                UVec3Size OBJIndex{};
                OBJIndex.X = OBJIndices[Index] - 1;
                OBJIndex.Y = OBJIndices[Index + 1] - 1;
                OBJIndex.Z = OBJIndices[Index + 2] - 1;
                auto ExistingOBJIndex = std::find(ScratchOBJIndices.begin(), ScratchOBJIndices.end(), OBJIndex);
                if (ExistingOBJIndex == ScratchOBJIndices.end())
                {
                    Positions.emplace_back(ScratchPositions[OBJIndex.X]);
                    TexCoords.emplace_back(ScratchTexCoords[OBJIndex.Y]);
                    Normals.emplace_back(ScratchNormals[OBJIndex.Z]);

                    ScratchOBJIndices.emplace_back(OBJIndex);

                    /** Add freshly added vertex index */
                    Indices.emplace_back(Positions.size() - 1);
                }
                else
                {
                    std::size_t ExistingIndex = std::distance(std::begin(ScratchOBJIndices), ExistingOBJIndex);
                    Indices.emplace_back(ExistingIndex);
                }
            }
        }
        else
        {
            /** Unused; Skip to next line */
            CurrentIndex = OBJContents.find('\n', CurrentIndex);
            if (CurrentIndex == std::string::npos)
            {
                break;
            }
        }

        CurrentIndex++;
    }
}

CRawImage::CRawImage(const SAsset& Resource)
{
    Data = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(Resource.Data), (int)Resource.Length,
        &Width,
        &Height, &Channels,
        4);
}

CRawImage::~CRawImage()
{
    stbi_image_free(Data);
}

CRawImageInfo::CRawImageInfo(const SAsset& Resource)
{
    auto Result = stbi_info_from_memory(reinterpret_cast<const stbi_uc*>(Resource.Data),
        (int)Resource.Length, &Width, &Height, &Channels);
    if (!Result)
    {
        abort();
    }
}
