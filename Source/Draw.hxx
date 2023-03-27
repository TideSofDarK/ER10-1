#pragma once

#include <vector>
#include <array>

#include "glm/glm.hpp"
#include "glm/mat4x4.hpp"
#include "CommonTypes.hxx"
#include "Asset.hxx"
#include "ShaderConstants.hxx"

#define RENDERER_QUEUE2D_SIZE 16
#define RENDERER_QUEUE3D_SIZE 8

#define TEXTURE_UNIT_TRANSIENT 0
#define TEXTURE_UNIT_ATLAS_COMMON 1
#define TEXTURE_UNIT_ATLAS_PRIMARY2D 2
#define TEXTURE_UNIT_ATLAS_PRIMARY3D 3
#define TEXTURE_UNIT_MAIN_FRAMEBUFFER 7

#define ATLAS_COUNT 4
#define ATLAS_MAX_SPRITE_COUNT 16
#define ATLAS_SIZE 1024
#define ATLAS_COMMON 0
#define ATLAS_PRIMARY2D 1
#define ATLAS_PRIMARY3D 2

struct SLevel;

struct SProgram {
private:
    static void CheckShader(unsigned ShaderID);

    static void CheckProgram(unsigned ShaderID);

    static unsigned int CreateVertexShader(const SResource *Resource);

    static unsigned int CreateFragmentShader(const SResource *Resource);

    static unsigned int CreateProgram(unsigned int VertexShader, unsigned int FragmentShader);

protected:
    virtual void InitUniforms();

public:
    unsigned ID{};
    int UniformModeID{};
    int UniformModeControlAID{};
    int UniformModeControlBID{};

    void
    Init(const SResource *VertexShaderData, const SResource *FragmentShaderData);

    void Cleanup() const;

    void Use() const;
};

struct SProgramPostProcess : SProgram {
protected:
    void InitUniforms() override;

public:
    int UniformColorTextureID{};
};

struct SProgram2D : SProgram {
protected:
    void InitUniforms() override;

public:
    int UniformBlockCommon2D{};
    int UniformPositionScreenSpaceID{};
    int UniformSizeScreenSpaceID{};
    int UniformUVRectID{};
    int UniformCommonAtlasID{};
    int UniformPrimaryAtlasID{};
};

struct SProgram3D : SProgram {
protected:
    void InitUniforms() override;

public:
    int UniformBlockCommon3D{};
    int UniformModelID{};
    int UniformCommonAtlasID{};
    int UniformPrimaryAtlasID{};
};

struct SFrameBuffer {
    unsigned FBO{};
    unsigned ColorID{};
    unsigned DepthID{};
    glm::vec3 ClearColor;

    void Init(int TextureUnitID, int Width, int Height, glm::vec3 InClearColor);

    void Cleanup();

    void BindForDrawing() const;

    void BindForReading() const;
};

struct SGeometry {
    unsigned VAO{};
    unsigned VBO{};
    unsigned EBO{};
    unsigned CBO{};
    int ElementCount{};

    void InitFromRawMesh(const CRawMesh &RawMesh);

    virtual void Cleanup();
};

struct SSubGeometry {
    int ElementOffset{};
    int ElementCount{};
};

namespace ETileGeometryType {
    enum {
        Floor,
        Wall,
        Ceil,
        Door,
        Hole,
        CustomA,
        CustomB,
        CustomC,
        Count
    };
}

struct STileset : SGeometry {
    /**  */
    std::array<SSubGeometry, 5> TileGeometry;

    void InitPlaceholder();

    void InitBasic(const SResource &Floor, const SResource &Wall, CScratchBuffer &ScratchBuffer);
};

struct SCamera {
    glm::mat4x4 View;
    glm::mat4x4 Projection;
    glm::vec3 Position;
    glm::vec3 Target;
    float Aspect{};
    float FieldOfViewY{};

    void Regenerate(float InFieldOfViewY, float InAspect);

    void Update();
};

struct STexture {
    unsigned ID{};

    void InitFromPixels(int Width, int Height, bool bAlpha, const void *Pixels);

    void InitEmpty(int Width, int Height, bool bAlpha);

    void InitFromRawImage(const CRawImage &RawImage);

    void Cleanup();

    void BindToTextureUnit(int TextureUnit) const;
};

struct SUniformBlock {
    unsigned UBO{};

    void Init(int Size);

    void Cleanup();

    void Bind() const;

    void SetMatrix(int Position, const glm::mat4x4 &Value) const;

    void SetVector2(int Position, const glm::vec2 &Value) const;

    void SetFloat(int Position, float Value) const;
};

enum class EProgram2DType {
    HUD,
    Uber2D,
};

struct SEntryMode {
    int ID{};
    glm::vec4 ControlA{};
    glm::vec4 ControlB{};
};

struct SEntry {
    SEntryMode Mode{};
};

struct SEntry2D : SEntry {
    EProgram2DType Program2DType{};
    glm::vec3 Position{};
    glm::vec2 SizePixels{};
    glm::vec4 UVRect{};
};

struct SInstancedDrawCall {
    SSubGeometry *SubGeometry{};
    std::array<glm::mat4x4, UBER3D_MODEL_COUNT> Transform{};
    int Count{};
};

template<int Size>
struct SInstancedDrawData {
    std::array<SInstancedDrawCall, Size> DrawCalls;
};

struct SEntry3D : SEntry {
    glm::mat4x4 Model{};
    SGeometry *Geometry{};
    SInstancedDrawCall *InstancedDrawCall{};
    int InstancedDrawCallCount{};
};

template<typename TEntry, int Size>
struct SRenderQueue {
    SUniformBlock CommonUniformBlock;
    int CurrentIndex{};
    std::array<TEntry, Size> Entries;

    void Enqueue(const TEntry &Entry) {
        if (CurrentIndex >= Entries.size()) {
            CurrentIndex = 0;
        }
        Entries[CurrentIndex] = Entry;
        CurrentIndex++;
    }

    void Reset() {
        CurrentIndex = 0;
    }

    void Init(int CommonUniformBlockSize) {
        CommonUniformBlock.Init(CommonUniformBlockSize);
    }

    void Cleanup() {
        CommonUniformBlock.Cleanup();
    }
};

struct SSpriteHandle {
    struct SAtlas *Atlas{};
    struct SSprite *Sprite{};
};

struct SSprite {
    glm::vec4 UVRect{};
    glm::vec<2, int> SizePixels{};
    const SResource *Resource{};
};

struct SAtlas : STexture {
private:
    static std::array<int, ATLAS_MAX_SPRITE_COUNT> SortingIndices;
    static constexpr int WidthAndHeight = ATLAS_SIZE;
    int CurrentIndex{};
    int TextureUnitID{};
public:
    std::array<SSprite, ATLAS_MAX_SPRITE_COUNT> Sprites;

    void Init(int InTextureUnitID);

    SSpriteHandle AddSprite(const SResource &Resource, CScratchBuffer &ScratchBuffer);

    void Build(CScratchBuffer &ScratchBuffer);
};

struct SRenderer {
    SRenderQueue<SEntry2D, RENDERER_QUEUE2D_SIZE> Queue2D;
    SRenderQueue<SEntry3D, RENDERER_QUEUE3D_SIZE> Queue3D;
    SAtlas Atlases[3];
    STileset Tileset;

    SProgram2D ProgramHUD;
    SProgram2D ProgramUber2D;
    SProgram3D ProgramUber3D;
    SProgramPostProcess ProgramPostProcess;

    SFrameBuffer MainFrameBuffer;
    SGeometry Quad2D;
    SInstancedDrawData<ETileGeometryType::Count> LevelDrawData;

    void Init(int Width, int Height);

    void Cleanup();

    void UploadProjectionAndViewFromCamera(const SCamera &Camera) const;

    void Flush(const SWindowData &WindowData);

#pragma region Queue_2D_API

    void DrawHUD(glm::vec3 Position, glm::vec2 Size, int Mode);

    void Draw2D(glm::vec3 Position, const SSpriteHandle &SpriteHandle);

    void Draw2DEx(glm::vec3 Position, const SSpriteHandle &SpriteHandle, int Mode, glm::vec4 ModeControlA);

    void Draw2DEx(glm::vec3 Position, const SSpriteHandle &SpriteHandle, int Mode, glm::vec4 ModeControlA,
                  glm::vec4 ModeControlB);

    void
    Draw2DHaze(glm::vec3 Position, const SSpriteHandle &SpriteHandle, float XIntensity, float YIntensity, float Speed);

    void Draw2DBackBlur(glm::vec3 Position, const SSpriteHandle &SpriteHandle, float Count, float Speed, float Step);

    void Draw2DGlow(glm::vec3 Position, const SSpriteHandle &SpriteHandle, glm::vec3 Color, float Intensity);

    void Draw2DDisintegrate(glm::vec3 Position, const SSpriteHandle &SpriteHandle, const SSpriteHandle &NoiseHandle,
                            float Progress);

#pragma endregion

#pragma region Queue_3D_API

    void Draw3D(glm::vec3 Position, SGeometry *Geometry);

    void Draw3DLevel(const SLevel &Level);

#pragma endregion
};
