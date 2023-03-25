#include "Resource.hxx"
#include "Utility.hxx"

#include <string>
#include <algorithm>
#include <array>

std::vector<glm::vec3> TempVertices(128);
std::vector<glm::vec2> TempTexCoords(64);
std::vector<glm::vec3> TempNormals(64);
std::vector<glm::vec<3, int>> TempOBJIndices(64);

void ParseMeshOBJ(const SResource &Resource,
                  SRawMesh &RawMesh) {
    TempVertices.clear();
    TempTexCoords.clear();
    TempNormals.clear();
    TempOBJIndices.clear();

    RawMesh.Positions.clear();
    RawMesh.Normals.clear();
    RawMesh.TexCoords.clear();

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
            ParseFloats(Data.data(), Data.data() + Data.size(), &Position[0], 3);
            TempVertices.emplace_back(Position);
        } else if (Token == "vn") {
            glm::vec3 Position{};
            ParseFloats(Data.data(), Data.data() + Data.size(), &Position[0], 3);
            TempNormals.emplace_back(Position);
        } else if (Token == "vt") {
            glm::vec2 Position{};
            ParseFloats(Data.data(), Data.data() + Data.size(), &Position[0], 2);
            TempTexCoords.emplace_back(Position);
        } else if (Token == "f") {
            std::array<int, 9> Indices{};
            ParseInts(Data.data(), Data.data() + Data.size(), &Indices[0], Indices.size());
            for (int Index = 0; Index < Indices.size(); Index += 3) {
                glm::vec<3, int> OBJIndex{};
                OBJIndex.x = Indices[Index] - 1;
                OBJIndex.y = Indices[Index + 1] - 1;
                OBJIndex.z = Indices[Index + 2] - 1;
                auto ExistingOBJIndex = std::find(TempOBJIndices.begin(), TempOBJIndices.end(), OBJIndex);
                if (ExistingOBJIndex == TempOBJIndices.end()) {
                    RawMesh.Positions.emplace_back(TempVertices[OBJIndex.x]);
                    RawMesh.TexCoords.emplace_back(TempTexCoords[OBJIndex.y]);
                    RawMesh.Normals.emplace_back(TempNormals[OBJIndex.z]);

                    TempOBJIndices.emplace_back(OBJIndex);

                    /** Add freshly added vertex index */
                    RawMesh.Indices.emplace_back(RawMesh.Positions.size() - 1);
                } else {
                    std::size_t ExistingIndex = std::distance(std::begin(TempOBJIndices), ExistingOBJIndex);
                    RawMesh.Indices.emplace_back(ExistingIndex);
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
