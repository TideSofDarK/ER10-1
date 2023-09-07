#include "AssetTools.hxx"

#define STR2(x) #x
#define STR(x) STR2(x)

#ifdef _WIN32
#define INCBIN_SECTION ".rdata, \"dr\""
#elif defined(__APPLE__)
#define INCBIN_SECTION "__TEXT,__const"
//#define INCBIN_SECTION ".const_data"
#else
#define INCBIN_SECTION ".rodata"
#endif

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

#ifdef __clang__
#define EXTERN_OR_INLINE extern
#else
#define EXTERN_OR_INLINE inline
#endif

#define DEFINE_ASSET(NAME, PATH) \
    INCBIN(NAME, PATH) \
    EXTERN_OR_INLINE const SAsset NAME(&(incbin_ ## NAME ## _start[0]), incbin_ ## NAME ## _length);

namespace Asset::Common {
    DEFINE_ASSET(FramePNG, "../Asset/Texture/Frame.png")
    DEFINE_ASSET(RefPNG, "../Asset/Texture/Ref.png")
    DEFINE_ASSET(AngelPNG, "../Asset/Texture/Angel.png")
    DEFINE_ASSET(NoisePNG, "../Asset/Texture/Noise.png")
    DEFINE_ASSET(QuadOBJ, "../Asset/Mesh/Quad.obj")
    DEFINE_ASSET(PillarOBJ, "../Asset/Mesh/Pillar.obj")
}

namespace Asset::Shader {
    DEFINE_ASSET(SharedGLSL, "../Asset/Shader/Shared.glsl")
    DEFINE_ASSET(HUDVERT, "../Asset/Shader/HUD.vert")
    DEFINE_ASSET(HUDFRAG, "../Asset/Shader/HUD.frag")
    DEFINE_ASSET(Uber2DVERT, "../Asset/Shader/Uber2D.vert")
    DEFINE_ASSET(Uber2DFRAG, "../Asset/Shader/Uber2D.frag")
    DEFINE_ASSET(Uber3DVERT, "../Asset/Shader/Uber3D.vert")
    DEFINE_ASSET(Uber3DFRAG, "../Asset/Shader/Uber3D.frag")
    DEFINE_ASSET(PostProcessVERT, "../Asset/Shader/PostProcess.vert")
    DEFINE_ASSET(PostProcessFRAG, "../Asset/Shader/PostProcess.frag")
}

namespace Asset::TileSet::Hotel {
    DEFINE_ASSET(FloorOBJ, "../Asset/TileSet/Hotel/Floor.obj")
    DEFINE_ASSET(WallOBJ, "../Asset/TileSet/Hotel/Wall.obj")
    DEFINE_ASSET(WallJointOBJ, "../Asset/TileSet/Hotel/WallJoint.obj")
    DEFINE_ASSET(DoorOBJ, "../Asset/TileSet/Hotel/Door.obj")
    DEFINE_ASSET(AtlasPNG, "../Asset/TileSet/Hotel/Atlas.png")
}
