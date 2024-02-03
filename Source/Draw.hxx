#pragma once

#include <array>
#include "CommonTypes.hxx"
#include "AssetTools.hxx"
#include "SharedConstants.hxx"
#include "Math.hxx"
#include "Tile.hxx"

#define RENDERER_QUEUE2D_SIZE 16
#define RENDERER_QUEUE3D_SIZE 8

#define TEXTURE_UNIT_TRANSIENT 0
#define TEXTURE_UNIT_ATLAS_COMMON 1
#define TEXTURE_UNIT_ATLAS_PRIMARY2D 2
#define TEXTURE_UNIT_ATLAS_PRIMARY3D 3
#define TEXTURE_UNIT_MAIN_FRAMEBUFFER 7
#define TEXTURE_UNIT_MAP_FRAMEBUFFER 8

#define ATLAS_COUNT 4
#define ATLAS_MAX_SPRITE_COUNT 16
#define ATLAS_SIZE 1024
#define ATLAS_COMMON 0
#define ATLAS_PRIMARY2D 1
#define ATLAS_PRIMARY3D 2

struct SLevel;

/* @TODO: Redesign so it's easier to upload. */
struct SShaderMapData
{
    int32_t Width{};
    int32_t Height{};
    float POVX{};
    float POVY{};
    std::array<STile, MAX_LEVEL_TILE_COUNT> Tiles{};
};

struct SShaderGlobals
{
    UVec2 ScreenSize;
    float Time{};
    float Random{};
};

struct SProgram
{
private:
    static void CheckShader(unsigned ShaderID);

    static void CheckProgram(unsigned ProgramID);

    static unsigned int CreateVertexShader(const char* Data, int Length);

    static unsigned int CreateFragmentShader(const char* Data, int Length);

    static unsigned int CreateProgram(unsigned int VertexShader, unsigned int FragmentShader);

protected:
    virtual void InitUniforms();

public:
    unsigned ID{};
    int UniformGlobals{};
    int UniformModeID{};
    int UniformModeControlAID{};
    int UniformModeControlBID{};

#ifdef EQUINOX_REACH_DEVELOPMENT
    const SAsset* VertexShaderAsset{};
    const SAsset* FragmentShaderAsset{};

    void Reload();
#endif

    void
    Init(const SAsset& InVertexShaderAsset, const SAsset& InFragmentShaderAsset);

    void Cleanup() const;

    void Use() const;
};

struct SProgramPostProcess : SProgram
{
protected:
    void InitUniforms() override;

public:
    int UniformColorTextureID{};
};

struct SProgram2D : SProgram
{
protected:
    void InitUniforms() override;

public:
    int UniformPositionScreenSpaceID{};
    int UniformSizeScreenSpaceID{};
};

struct SProgramUber2D : SProgram2D
{
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

struct SProgramHUD : SProgram2D
{
protected:
    void InitUniforms() override;
};

struct SProgramMap : SProgram2D
{
protected:
    void InitUniforms() override;

public:
    int UniformMap{};
};

struct SProgram3D : SProgram
{
protected:
    void InitUniforms() override;

public:
    int UniformModelID{};
    int UniformCommonAtlasID{};
    int UniformPrimaryAtlasID{};
};

struct SFramebuffer
{
    int Width{};
    int Height{};
    unsigned FBO{};
    unsigned ColorID{};
    unsigned DepthID{};
    UVec3 ClearColor{};

    void Init(int TextureUnitID, int InWidth, int InHeight, UVec3 InClearColor, bool bLinearFiltering = false);

    void Cleanup();

    void ResetViewport() const;

    void BindForDrawing() const;

    void BindForReading() const;

    static void Unbind();
};

struct SGeometry
{
    unsigned VAO{};
    unsigned VBO{};
    unsigned EBO{};
    unsigned CBO{};
    int ElementCount{};

    void InitFromRawMesh(const CRawMesh& RawMesh);

    virtual void Cleanup();
};

struct SSubGeometry
{
    int ElementOffset{};
    int ElementCount{};
};

namespace ETileGeometryType
{
    enum
    {
        Floor,
        Hole,
        Wall,
        WallJoint,
        Ceil,
        Door,
        DoorFrame,
        CustomA,
        CustomB,
        CustomC,
        Count
    };
}

enum class EDoorAnimationType
{
    None,
    OneDoor,
    TwoDoors,
    Curtains
};

struct STileset : SGeometry
{
    EDoorAnimationType DoorAnimationType{};
    float DoorOffset{};
    std::array<SSubGeometry, ETileGeometryType::Count> TileGeometry;

    void InitPlaceholder();

    void InitBasic(
        const SAsset& Floor,
        const SAsset& Hole,
        const SAsset& Wall,
        const SAsset& WallJoint,
        const SAsset& DoorFrame,
        const SAsset& Door);
};

struct SCamera
{
    UMat4x4 View{};
    UMat4x4 Projection{};
    UVec3 Position{};
    UVec3 Target{};

    float Aspect = 2.4f;
    float FieldOfViewY = 77.7f;
    float ZNear = 0.01f;
    float ZFar = 100.0f;

    void RegenerateProjection();

    void Update();
};

struct STexture
{
    unsigned ID{};

    void InitFromPixels(int Width, int Height, bool bAlpha, const void* Pixels);

    void InitEmpty(int Width, int Height, bool bAlpha);

    void InitFromRawImage(const CRawImage& RawImage);

    void Cleanup();

    void BindToTextureUnit(int TextureUnit) const;
};

struct SUniformBlock
{
    unsigned UBO{};
    int Binding{};

    void Init(int Size);

    void Cleanup();

    void Bind(int BindingPoint) const;

    void SetMatrix(int Position, const UMat4x4& Value) const;

    void SetVector2(int Position, const UVec2& Value) const;

    void SetFloat(int Position, float Value) const;
};

enum class EProgram2DType
{
    HUD,
    Uber2D,
    Map
};

struct SEntryMode
{
    int ID{};
    UVec4 ControlA{};
    UVec4 ControlB{};
};

struct SEntry
{
    SEntryMode Mode{};
};

struct SEntry2D : SEntry
{
    EProgram2DType Program2DType{};
    UVec3 Position{};
    UVec2Int SizePixels{};
    UVec4 UVRect{};
};

struct SInstancedDrawCall
{
    const SSubGeometry* SubGeometry{};
    std::array<UMat4x4, UBER3D_MODEL_COUNT> Transform{};
    int Count{};
    int DynamicCount{};
    void Push(const UMat4x4& NewTransform)
    {
        Transform[Count % UBER3D_MODEL_COUNT] = NewTransform;
        Count++;
    }
    void PushDynamic(const UMat4x4& NewTransform)
    {
        Transform[(Count + DynamicCount) % UBER3D_MODEL_COUNT] = NewTransform;
        DynamicCount++;
    }
};

template <int Size>
struct SInstancedDrawData
{
    const STileset* TileSet{};
    std::array<SInstancedDrawCall, Size> DrawCalls;

    void Clear()
    {
        for (auto& DrawCall : DrawCalls)
        {
            DrawCall.Count = 0;
        }
    }
};

struct SEntry3D : SEntry
{
    UMat4x4 Model{};
    const SGeometry* Geometry{};
    SInstancedDrawCall* InstancedDrawCall{};
    int InstancedDrawCallCount{};
};

template <typename TEntry, int Size>
struct SRenderQueue
{
    SUniformBlock CommonUniformBlock;
    int CurrentIndex{};
    std::array<TEntry, Size> Entries;

    void Enqueue(const TEntry& Entry)
    {
        if (CurrentIndex >= (int)Entries.size())
        {
            CurrentIndex = 0;
        }
        Entries[CurrentIndex] = Entry;
        CurrentIndex++;
    }

    void Reset()
    {
        CurrentIndex = 0;
    }
};

struct SSpriteHandle
{
    struct SAtlas* Atlas{};
    struct SSprite* Sprite{};
};

struct SSprite
{
    UVec4 UVRect{};
    UVec2Int SizePixels{};
    const SAsset* Resource{};
};

struct SAtlas : STexture
{
private:
    static inline std::array<int, ATLAS_MAX_SPRITE_COUNT> SortingIndices;
    static constexpr int WidthAndHeight = ATLAS_SIZE;
    int CurrentIndex{};
    int TextureUnitID{};

public:
    std::array<SSprite, ATLAS_MAX_SPRITE_COUNT> Sprites;

    void Init(int InTextureUnitID);

    SSpriteHandle AddSprite(const SAsset& Resource);

    void Build();
};

struct SRenderer
{
    SRenderQueue<SEntry2D, RENDERER_QUEUE2D_SIZE> Queue2D;
    SRenderQueue<SEntry3D, RENDERER_QUEUE3D_SIZE> Queue3D;
    SAtlas Atlases[3];

    SUniformBlock GlobalsUniformBlock;
    SUniformBlock MapUniformBlock;

    SProgramHUD ProgramHUD;
    SProgramMap ProgramMap;
    SProgramUber2D ProgramUber2D;
    SProgram3D ProgramUber3D;
    SProgramPostProcess ProgramPostProcess;

    SFramebuffer MainFrameBuffer;
    SGeometry Quad2D;
    SInstancedDrawData<ETileGeometryType::Count> LevelDrawData;

    void Init(int Width, int Height);

    void Cleanup();

    void SetupLevelDrawData(const STileset& TileSet);

    void BindMapUniformBlock(const SUniformBlock* UniformBlock) const;
    void UploadMapData(const SLevel& Level, UVec2 POVOrigin, const SUniformBlock* UniformBlock) const;

    void UploadProjectionAndViewFromCamera(const SCamera& Camera) const;

    void Flush(const SWindowData& WindowData);

#pragma region Queue_2D_API

    void DrawHUD(UVec3 Position, UVec2Int Size, int Mode);

    void DrawMap(SLevel& Level, UVec3 Position, UVec2Int Size, const UVec2& POVOrigin);

    void DrawMapImmediate(const UVec2& Position, const UVec2Int& Size, const UVec2& ScreenSize, float Time);

    void Draw2D(UVec3 Position, const SSpriteHandle& SpriteHandle);

    void Draw2DEx(UVec3 Position, const SSpriteHandle& SpriteHandle, int Mode, UVec4 ModeControlA);

    void Draw2DEx(UVec3 Position, const SSpriteHandle& SpriteHandle, int Mode, UVec4 ModeControlA,
        UVec4 ModeControlB);

    void
    Draw2DHaze(UVec3 Position, const SSpriteHandle& SpriteHandle, float XIntensity, float YIntensity, float Speed);

    void Draw2DBackBlur(UVec3 Position, const SSpriteHandle& SpriteHandle, float Count, float Speed, float Step);

    void Draw2DGlow(UVec3 Position, const SSpriteHandle& SpriteHandle, UVec3 Color, float Intensity);

    void Draw2DDisintegrate(UVec3 Position, const SSpriteHandle& SpriteHandle, const SSpriteHandle& NoiseHandle,
        float Progress);

#pragma endregion

#pragma region Queue_3D_API

    void Draw3D(UVec3 Position, SGeometry* Geometry);

    void Draw3DLevel(SLevel& Level, const UVec2Int& POVOrigin, const SDirection& POVDirection);

    void Draw3DLevelDoor(SInstancedDrawCall& DoorDrawCall, const UVec2Int& TileCoords, SDirection Direction, float AnimationAlpha = 0.0f) const;

#pragma endregion
};
