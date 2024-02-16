#pragma once

#include <array>
#include "CommonTypes.hxx"
#include "AssetTools.hxx"
#include "SharedConstants.hxx"
#include "Math.hxx"
#include "Tile.hxx"
#include "Utility.hxx"

#define RENDERER_QUEUE2D_SIZE 16
#define RENDERER_QUEUE3D_SIZE 8

#define ATLAS_COUNT 4
#define ATLAS_MAX_SPRITE_COUNT 16
#define ATLAS_COMMON 0
#define ATLAS_PRIMARY2D 1
#define ATLAS_PRIMARY3D 2

namespace ETextureUnits
{
    enum
    {
        Transient,
        AtlasCommon,
        AtlasPrimary2D,
        AtlasPrimary3D,
        MainFramebuffer,
        MapFramebuffer,
        WorldTextures
    };
}

inline constexpr SVec2 MapTextureSize{
    Utility::NextPowerOfTwo(MAP_MAX_WIDTH_PIXELS),
    Utility::NextPowerOfTwo(MAP_MAX_HEIGHT_PIXELS)
};

inline constexpr SVec2 MapWorldLayerTextureSize{
    Utility::NextPowerOfTwo(MAP_ISO_MAX_WIDTH_PIXELS),
    Utility::NextPowerOfTwo(MAP_ISO_MAX_WIDTH_PIXELS)
};

struct SWorldLevel;

/* @TODO: Redesign so it's easier to upload. */
struct SShaderMapData
{
    int32_t Width{};
    int32_t Height{};
    SCoordsAndDirection POV;
    int : 32;
    int : 32;
    int : 32;
    std::array<STile, MAX_LEVEL_TILE_COUNT> Tiles{};
};

struct SShaderWorld
{
    struct SShaderWorldLayer
    {
        SVec3 Color{};
        uint32_t Index{};
        SVec2 Position{};
        SVec2 TextureSize{};
    };
    SVec4 Position;
    SShaderWorldLayer Layers[WORLD_MAX_LAYERS];
};

struct SShaderGlobals
{
    SVec2 ScreenSize;
    float Time{};
    float Random{};
};

struct SShaderSprite
{
    SVec4 UVRect{};
    int SizeX{};
    int SizeY{};
    int : 32;
    int : 32;
};

struct SShaderMapCommon
{
    float Editor{};
    SVec3 PaddingA{};
    SShaderSprite Icons[MAP_ICON_COUNT];
};

struct SUniformBlock
{
    unsigned UBO{};
    int Binding{};

    void Init(int Size);

    void Cleanup() const;

    void Bind(int BindingPoint) const;

    void SetMatrix(int Position, const SMat4x4& Value) const;

    void SetVector2(int Position, const SVec2& Value) const;

    void SetFloat(int Position, float Value) const;
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
    virtual void InitUniformBlocks(){};
    virtual void CleanupUniformBlocks() const {};

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
    int UniformCommonAtlasID{};
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
    void InitUniformBlocks() override;
    void CleanupUniformBlocks() const override;

public:
    SUniformBlock UniformBlockCommon{};
    SUniformBlock UniformBlockMap{};
    SUniformBlock UniformBlockWorld{};
    int UniformCommonAtlasID{};
    int UniformWorldTextures{};
    int UniformWorldLayers{};
    int UniformMap{};
    int UniformCursor{};

    void SetEditor(bool bEditor) const;
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

struct SWorldFramebuffer
{
    int Width{};
    int Height{};
    unsigned FBO{};
    unsigned ColorID{};
    SVec3 ClearColor{};

    void Init(int TextureUnitID, int InWidth, int InHeight, SVec3 InClearColor, bool bLinearFiltering = false);

    void Cleanup();

    void ResetViewport() const;

    void SetLayer(int LayerIndex) const;
};

struct SFramebuffer
{
    int Width{};
    int Height{};
    unsigned FBO{};
    unsigned ColorID{};
    unsigned DepthID{};
    SVec3 ClearColor{};

    void Init(int TextureUnitID, int InWidth, int InHeight, SVec3 InClearColor, bool bLinearFiltering = false);

    void Cleanup();

    void ResetViewport() const;
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
    SMat4x4 View{};
    SMat4x4 Projection{};
    SVec3 Position{};
    SVec3 Target{};

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

enum class EProgram2DType
{
    HUD,
    Uber2D,
    Map
};

struct SEntryMode
{
    int ID{};
    SVec4 ControlA{};
    SVec4 ControlB{};
};

struct SEntry
{
    SEntryMode Mode{};
};

struct SEntry2D : SEntry
{
    EProgram2DType Program2DType{};
    SVec3 Position{};
    SVec2Int SizePixels{};
    SVec4 UVRect{};
};

struct SInstancedDrawCall
{
    const SSubGeometry* SubGeometry{};
    std::array<SMat4x4, UBER3D_MODEL_COUNT> Transform{};
    int Count{};
    int DynamicCount{};
    void Push(const SMat4x4& NewTransform)
    {
        Transform[Count % UBER3D_MODEL_COUNT] = NewTransform;
        Count++;
    }
    void PushDynamic(const SMat4x4& NewTransform)
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
    SMat4x4 Model{};
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
    SVec4 UVRect{};
    SVec2Int SizePixels{};
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

    SProgramHUD ProgramHUD;
    SProgramMap ProgramMap;
    SProgramUber2D ProgramUber2D;
    SProgram3D ProgramUber3D;
    SProgramPostProcess ProgramPostProcess;

    SFramebuffer MainFramebuffer;
    SFramebuffer MapFramebuffer;
    SWorldFramebuffer WorldLayersFramebuffer;
    SGeometry Quad2D;
    SInstancedDrawData<ETileGeometryType::Count> LevelDrawData;

    void Init(int Width, int Height);

    void Cleanup();

    void SetupTileset(const STileset* TileSet);

    /* Map */
    void SetMapIcons(const std::array<SSpriteHandle, MAP_ICON_COUNT>& SpriteHandles) const;
    void UploadMapData(const SWorldLevel* Level, const SCoordsAndDirection& POV) const;

    void UploadProjectionAndViewFromCamera(const SCamera& Camera) const;

    void SetTime(float Time) const;

    void Flush(const SWindowData& WindowData);

#pragma region Queue_2D_API

    void DrawHUD(SVec3 Position, SVec2Int Size, int Mode);

    void DrawMap(SWorldLevel* Level, SVec3 Position, SVec2Int Size, const SCoordsAndDirection& POV);

    void DrawMapImmediate(const SVec2& Position, const SVec2& Size);

    void DrawWorldMap(const SVec2& Position, const SVec2& Size);

    void DrawWorldMapImmediate(const SVec2& Position, const SVec2& Size);

    void DrawWorldLayers(const struct SWorld* World, SVec2Int Range);

    void Draw2D(SVec3 Position, const SSpriteHandle& SpriteHandle);

    void Draw2DEx(SVec3 Position, const SSpriteHandle& SpriteHandle, int Mode, SVec4 ModeControlA);

    void Draw2DEx(SVec3 Position, const SSpriteHandle& SpriteHandle, int Mode, SVec4 ModeControlA,
        SVec4 ModeControlB);

    void
    Draw2DHaze(SVec3 Position, const SSpriteHandle& SpriteHandle, float XIntensity, float YIntensity, float Speed);

    void Draw2DBackBlur(SVec3 Position, const SSpriteHandle& SpriteHandle, float Count, float Speed, float Step);

    void Draw2DGlow(SVec3 Position, const SSpriteHandle& SpriteHandle, SVec3 Color, float Intensity);

    void Draw2DDisintegrate(SVec3 Position, const SSpriteHandle& SpriteHandle, const SSpriteHandle& NoiseHandle,
        float Progress);

#pragma endregion

#pragma region Queue_3D_API

    void Draw3D(SVec3 Position, SGeometry* Geometry);

    void Draw3DLevel(SWorldLevel* Level, const SVec2Int& POVOrigin, const SDirection& POVDirection);

    void Draw3DLevelDoor(SInstancedDrawCall& DoorDrawCall, const SVec2Int& TileCoords, SDirection Direction, float AnimationAlpha = 0.0f) const;

#pragma endregion
};
