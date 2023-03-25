#include "Resource.hxx"

#include <string>
#include <algorithm>
#include <array>

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

#include "Utility.hxx"

std::vector<glm::vec3> TempVertices(128);
std::vector<glm::vec2> TempTexCoords(64);
std::vector<glm::vec3> TempNormals(64);
std::vector<glm::vec<3, int>> TempOBJIndices(64);

CRawMesh::CRawMesh(const SResource &Resource) {
    TempVertices.clear();
    TempTexCoords.clear();
    TempNormals.clear();
    TempOBJIndices.clear();

    Positions.clear();
    Normals.clear();
    TexCoords.clear();

    auto OBJLength = static_cast<size_t>(Resource.Length);
    std::string_view OBJContents{reinterpret_cast<const char *>(Resource.Data), OBJLength};

    size_t CurrentIndex{};
    while (CurrentIndex < OBJLength) {
        /** Each line is "$token $data" e. g. "v 1.0 1.0 1.0" */
        auto TokenEndIndex = OBJContents.find(' ', CurrentIndex);
        auto Token = std::string_view{&OBJContents[CurrentIndex], TokenEndIndex - CurrentIndex};
        CurrentIndex = TokenEndIndex;
        auto DataEndIndex = OBJContents.find('\n', CurrentIndex);
        std::string_view Data{&OBJContents[CurrentIndex + 1], DataEndIndex - CurrentIndex};
        CurrentIndex = DataEndIndex;
        if (Token == "v") {
            glm::vec3 Position{};
            Utility::ParseFloats(Data.data(), Data.data() + Data.size(), &Position[0], 3);
            TempVertices.emplace_back(Position);
        } else if (Token == "vn") {
            glm::vec3 Position{};
            Utility::ParseFloats(Data.data(), Data.data() + Data.size(), &Position[0], 3);
            TempNormals.emplace_back(Position);
        } else if (Token == "vt") {
            glm::vec2 Position{};
            Utility::ParseFloats(Data.data(), Data.data() + Data.size(), &Position[0], 2);
            TempTexCoords.emplace_back(Position);
        } else if (Token == "f") {
            std::array<int, 9> OBJIndices{};
            Utility::ParseInts(Data.data(), Data.data() + Data.size(), &OBJIndices[0], OBJIndices.size());
            for (int Index = 0; Index < OBJIndices.size(); Index += 3) {
                glm::vec<3, int> OBJIndex{};
                OBJIndex.x = OBJIndices[Index] - 1;
                OBJIndex.y = OBJIndices[Index + 1] - 1;
                OBJIndex.z = OBJIndices[Index + 2] - 1;
                auto ExistingOBJIndex = std::find(TempOBJIndices.begin(), TempOBJIndices.end(), OBJIndex);
                if (ExistingOBJIndex == TempOBJIndices.end()) {
                    Positions.emplace_back(TempVertices[OBJIndex.x]);
                    TexCoords.emplace_back(TempTexCoords[OBJIndex.y]);
                    Normals.emplace_back(TempNormals[OBJIndex.z]);

                    TempOBJIndices.emplace_back(OBJIndex);

                    /** Add freshly added vertex index */
                    Indices.emplace_back(Positions.size() - 1);
                } else {
                    std::size_t ExistingIndex = std::distance(std::begin(TempOBJIndices), ExistingOBJIndex);
                    Indices.emplace_back(ExistingIndex);
                }
            }
        } else {
            /** Unused; Skip to next line */
            CurrentIndex = OBJContents.find('\n', CurrentIndex);
            if (CurrentIndex == std::string::npos) {
                break;
            }
        }

        CurrentIndex++;
    }
}

CRawImage::CRawImage(const SResource &Resource) {
    Data = stbi_load_from_memory(Resource.Data, static_cast<int>(Resource.Length),
                                 &Width,
                                 &Height, &Channels,
                                 4);
}

CRawImage::~CRawImage() {
    stbi_image_free(Data);
}

CRawImageInfo::CRawImageInfo(const SResource &Resource) {
    auto Result = stbi_info_from_memory(Resource.Data, static_cast<int>(Resource.Length), &Width, &Height, &Channels);
    if (!Result) {
        abort();
    }
}
