#include "AssetTools.hxx"

#define STR2(x) #x
#define STR(x) STR2(x)

#ifdef _WIN32
    #define INCBIN_SECTION ".rdata, \"dr\""
#elif defined(__APPLE__)
    #define INCBIN_SECTION "__TEXT,__const"
// #define INCBIN_SECTION ".const_data"
#else
    #define INCBIN_SECTION ".rodata"
#endif

// clang-format off
#ifdef __APPLE__
#define INCBIN(name, file) \
    __asm__(".section " INCBIN_SECTION "\n" \
            ".global " "_incbin" "_" STR(name) "_start\n" \
            ".balign 16\n" \
            "_incbin" "_" STR(name) "_start:\n" \
            ".incbin \"" file "\"\n" \
            \
            ".global " "_incbin" "_" STR(name) "_end\n" \
            ".balign 1\n" \
            "_incbin" "_" STR(name) "_end:\n" \
            ".byte 0\n" \
    ); \
    extern "C" __attribute__((aligned(16))) const char incbin ## _ ## name ## _start[]; \
    extern "C"                              const char incbin ## _ ## name ## _end[]; \
    const size_t incbin_ ## name ## _length = incbin_ ## name ## _end - incbin_ ## name ## _start;
#else
#define INCBIN(name, file) \
    __asm__(".section " INCBIN_SECTION "\n" \
            ".global " STR(__USER_LABEL_PREFIX__) "incbin_" STR(name) "_start\n" \
            ".balign 16\n" \
            STR(__USER_LABEL_PREFIX__)"incbin_" STR(name) "_start:\n" \
            ".incbin \"" file "\"\n" \
            \
            ".global " STR(__USER_LABEL_PREFIX__) "incbin_" STR(name) "_end\n" \
            ".balign 1\n" \
            STR(__USER_LABEL_PREFIX__)"incbin_" STR(name) "_end:\n" \
            ".byte 0\n" \
    ); \
    extern "C" __attribute__((aligned(16))) const char incbin_ ## name ## _start[]; \
    extern "C"                              const char incbin_ ## name ## _end[]; \
    const size_t incbin_ ## name ## _length = incbin_ ## name ## _end - incbin_ ## name ## _start;
#endif
// clang-format on

#ifdef __clang__
    #define EXTERN_OR_INLINE extern
#else
    #define EXTERN_OR_INLINE inline
#endif

#ifdef EQUINOX_REACH_DEVELOPMENT
    #define DEFINE_ASSET(NAME, PATH)                  \
        INCBIN(NAME, EQUINOX_REACH_ASSET_PATH PATH) \
        EXTERN_OR_INLINE const SAsset NAME(&(incbin_##NAME##_start[0]), incbin_##NAME##_length, PATH);
#else
    #define DEFINE_ASSET(NAME, PATH)                  \
        INCBIN(NAME, EQUINOX_REACH_ASSET_PATH PATH ) \
        EXTERN_OR_INLINE const SAsset NAME(&(incbin_##NAME##_start[0]), incbin_##NAME##_length);
#endif

namespace Asset::Common
{
    DEFINE_ASSET(FramePNG, "Texture/Frame.png")
    DEFINE_ASSET(RefPNG, "Texture/Ref.png")
    DEFINE_ASSET(AngelPNG, "Texture/Angel.png")
    DEFINE_ASSET(NoisePNG, "Texture/Noise.png")
    DEFINE_ASSET(QuadOBJ, "Mesh/Quad.obj")
    DEFINE_ASSET(PillarOBJ, "Mesh/Pillar.obj")
    DEFINE_ASSET(IBMPlexSansTTF, "Font/IBMPlexSans.ttf")
    DEFINE_ASSET(DoorCreekWAV, "Sound/DoorCreek.wav")
    DEFINE_ASSET(Tile_01WAV, "Sound/Step/Tile_01.wav")
    DEFINE_ASSET(TestMusicWAV, "Music/TestMusic.wav")
}

namespace Asset::HUD
{
    /* Map Icons */
    DEFINE_ASSET(MapIconPlayer, "HUD/Player.png")
    DEFINE_ASSET(MapIconA, "HUD/IconA.png")
    DEFINE_ASSET(MapIconB, "HUD/IconB.png")
    DEFINE_ASSET(MapIconHole, "HUD/Hole.png")
}

namespace Asset::Shader
{
    DEFINE_ASSET(SharedGLSL, "Shader/Shared.glsl")
    DEFINE_ASSET(HUDVERT, "Shader/HUD.vert")
    DEFINE_ASSET(HUDFRAG, "Shader/HUD.frag")
    DEFINE_ASSET(MapVERT, "Shader/Map.vert")
    DEFINE_ASSET(MapFRAG, "Shader/Map.frag")
    DEFINE_ASSET(Uber2DVERT, "Shader/Uber2D.vert")
    DEFINE_ASSET(Uber2DFRAG, "Shader/Uber2D.frag")
    DEFINE_ASSET(Uber3DVERT, "Shader/Uber3D.vert")
    DEFINE_ASSET(Uber3DFRAG, "Shader/Uber3D.frag")
    DEFINE_ASSET(PostProcessVERT, "Shader/PostProcess.vert")
    DEFINE_ASSET(PostProcessFRAG, "Shader/PostProcess.frag")
}

namespace Asset::Map
{
    DEFINE_ASSET(TestMapERM, "Map/TestMap.erm")
    DEFINE_ASSET(Floor0, "Map/Floor0.erm")
    DEFINE_ASSET(Floor1, "Map/Floor1.erm")
    DEFINE_ASSET(Floor2, "Map/Floor2.erm")
    DEFINE_ASSET(Floor3, "Map/Floor3.erm")
}

namespace Asset::Tileset::Hotel
{
    DEFINE_ASSET(FloorOBJ, "Tileset/Hotel/Floor.obj")
    DEFINE_ASSET(HoleOBJ, "Tileset/Hotel/Hole.obj")
    DEFINE_ASSET(WallOBJ, "Tileset/Hotel/Wall.obj")
    DEFINE_ASSET(WallJointOBJ, "Tileset/Hotel/WallJoint.obj")
    DEFINE_ASSET(DoorFrameOBJ, "Tileset/Hotel/DoorFrame.obj")
    DEFINE_ASSET(DoorOBJ, "Tileset/Hotel/Door.obj")
    DEFINE_ASSET(AtlasPNG, "Tileset/Hotel/Atlas.png")
}
