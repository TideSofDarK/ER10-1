#include "Draw.hpp"

#include <iostream>
#include <algorithm>
#include <numeric>
#include <string>
#include "glad/gl.h"
#include <glm/gtc/matrix_transform.hpp>
#include "Level.hpp"
#include "Constants.hpp"
#include "Utility.hpp"

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

static std::string const GLSLVersion = "#version 410 core\n";
static std::string const ShaderConstants{
#define SHADER_CONSTANTS_LITERAL

#include "ShaderConstants.hpp"

#undef SHADER_CONSTANTS_LITERAL
};

DEFINE_RESOURCE(Shared_glsl)
DEFINE_RESOURCE(HUD_vert)
DEFINE_RESOURCE(HUD_frag)
DEFINE_RESOURCE(Uber2D_vert)
DEFINE_RESOURCE(Uber2D_frag)
DEFINE_RESOURCE(Uber3D_vert)
DEFINE_RESOURCE(Uber3D_frag)
DEFINE_RESOURCE(PostProcess_vert)
DEFINE_RESOURCE(PostProcess_frag)

void SProgram::CheckShader(unsigned int ShaderID) {
    int Success;
    glGetShaderiv(ShaderID, GL_COMPILE_STATUS, &Success);
    if (!Success) {
        GLint MaxLength = 0;
        glGetShaderiv(ShaderID, GL_INFO_LOG_LENGTH, &MaxLength);
        GLsizei LengthQuery(0);
        std::vector<GLchar> InfoLog(MaxLength + 1, '\0');
        glGetShaderInfoLog(ShaderID, GLsizei(InfoLog.size()), &LengthQuery, &InfoLog[0]);
        std::cout << std::string(InfoLog.begin(), InfoLog.end()) << std::endl;
    }
}

void SProgram::CheckProgram(unsigned int ProgramID) {
    int Success;
    glValidateProgram(ProgramID);
    glGetProgramiv(ProgramID, GL_VALIDATE_STATUS, &Success);
    if (!Success) {
        GLint MaxLength = 0;
        glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &MaxLength);
        GLsizei LengthQuery(0);
        std::vector<GLchar> InfoLog(MaxLength + 1, '\0');
        glGetProgramInfoLog(ProgramID, GLsizei(InfoLog.size()), &LengthQuery, &InfoLog[0]);
        std::cout << std::string(InfoLog.begin(), InfoLog.end()) << std::endl;
    }
}

unsigned int SProgram::CreateVertexShader(const SResource *Resource) {
    unsigned VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    char const *Blocks[4] = {
            &GLSLVersion[0],
            reinterpret_cast<const char *>(ResourceShared_glsl.Ptr),
            &ShaderConstants[0],
            reinterpret_cast<const char *>(Resource->Ptr)
    };
    int const Lengths[4] = {
            static_cast<int>(GLSLVersion.length()),
            static_cast<int>(ResourceShared_glsl.Length),
            static_cast<int>(ShaderConstants.length()),
            static_cast<int>(Resource->Length)
    };
    glShaderSource(VertexShaderID, 4, Blocks, &Lengths[0]);
    glCompileShader(VertexShaderID);
    CheckShader(VertexShaderID);
    return VertexShaderID;
}

unsigned int SProgram::CreateFragmentShader(const SResource *Resource) {
    unsigned FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    char const *Blocks[4] = {
            &GLSLVersion[0],
            reinterpret_cast<const char *>(ResourceShared_glsl.Ptr),
            &ShaderConstants[0],
            reinterpret_cast<const char *>(Resource->Ptr)
    };
    int const Lengths[4] = {
            static_cast<int>(GLSLVersion.length()),
            static_cast<int>(ResourceShared_glsl.Length),
            static_cast<int>(ShaderConstants.length()),
            static_cast<int>(Resource->Length)
    };
    glShaderSource(FragmentShaderID, 4, Blocks, &Lengths[0]);
    glCompileShader(FragmentShaderID);
    CheckShader(FragmentShaderID);
    return FragmentShaderID;
}

unsigned int SProgram::CreateProgram(unsigned int VertexShader, unsigned int FragmentShader) {
    unsigned ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShader);
    glAttachShader(ProgramID, FragmentShader);
    glLinkProgram(ProgramID);
    CheckProgram(ProgramID);
    return ProgramID;
}

void SProgram::InitUniforms() {
    UniformModeID = glGetUniformLocation(ID, "u_mode");
    UniformModeControlAID = glGetUniformLocation(ID, "u_modeControlA");
    UniformModeControlBID = glGetUniformLocation(ID, "u_modeControlB");
}

void SProgram::Init(const SResource *VertexShaderData, const SResource *FragmentShaderData) {
    unsigned VertexShader = CreateVertexShader(VertexShaderData);
    unsigned FragmentShader = CreateFragmentShader(FragmentShaderData);
    ID = CreateProgram(VertexShader, FragmentShader);
    glDeleteShader(VertexShader);
    glDeleteShader(FragmentShader);
    SProgram::InitUniforms();
    InitUniforms();
}

void SProgram::Cleanup() const {
    glDeleteProgram(ID);

    std::cout << "Deleting SProgram..." << std::endl;
}

void SProgram::Use() const {
    glUseProgram(ID);
}

void SProgramPostProcess::InitUniforms() {
    UniformColorTextureID = glGetUniformLocation(ID, "u_colorTexture");
}

void SProgram3D::InitUniforms() {
    UniformBlockCommon3D = glGetUniformBlockIndex(ID, "ub_common");
    glUniformBlockBinding(ID, UniformBlockCommon3D, 0);
    UniformModelID = glGetUniformLocation(ID, "u_model");
    UniformCommonAtlasID = glGetUniformLocation(ID, "u_commonAtlas");
    UniformPrimaryAtlasID = glGetUniformLocation(ID, "u_primaryAtlas");
}

void SProgram2D::InitUniforms() {
    UniformBlockCommon2D = glGetUniformBlockIndex(ID, "ub_common");
    glUniformBlockBinding(ID, UniformBlockCommon2D, 0);
    UniformPositionScreenSpaceID = glGetUniformLocation(ID, "u_positionScreenSpace");
    UniformSizeScreenSpaceID = glGetUniformLocation(ID, "u_sizeScreenSpace");
    UniformUVRectID = glGetUniformLocation(ID, "u_uvRect");
    UniformCommonAtlasID = glGetUniformLocation(ID, "u_commonAtlas");
    UniformPrimaryAtlasID = glGetUniformLocation(ID, "u_primaryAtlas");
}

void SGeometry::Cleanup() {
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &CBO);
    glDeleteVertexArrays(1, &VAO);

    std::cout << "Deleting SGeometry with ElementCount == " + std::to_string(ElementCount) << std::endl;
}

void STileset::InitPlaceholder() {
    std::array<glm::vec3, 8> TempVertices{};
    std::array<unsigned short, 12> Indices{};

    /** Floor Quad */
    auto &FloorGeometry = TileGeometry[ETileGeometryType::Floor];
    FloorGeometry.ElementOffset = 0;
    FloorGeometry.ElementCount = 6;
    Indices[0] = 0;
    Indices[1] = 1;
    Indices[2] = 2;
    Indices[3] = 0;
    Indices[4] = 2;
    Indices[5] = 3;

    TempVertices[0] = {0.5f, 0.0f, 0.5f};
    TempVertices[1] = {0.5f, 0.0f, -0.5f};
    TempVertices[2] = {-0.5f, 0.0f, -0.5f};
    TempVertices[3] = {-0.5f, 0.0f, 0.5f};

    /** Wall Quad */
    auto &WallGeometry = TileGeometry[ETileGeometryType::Wall];
    WallGeometry.ElementOffset = 12;
    WallGeometry.ElementCount = 6;
    Indices[6] = 4 + 0;
    Indices[7] = 4 + 1;
    Indices[8] = 4 + 2;
    Indices[9] = 4 + 0;
    Indices[10] = 4 + 2;
    Indices[11] = 4 + 3;

    TempVertices[4] = {0.5f, 1.0f, -0.5f};
    TempVertices[5] = {-0.5f, 1.0f, -0.5f};
    TempVertices[6] = {-0.5f, 0.0f, -0.5f};
    TempVertices[7] = {0.5f, 0.0f, -0.5f};

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, static_cast<long long>(TempVertices.size() * sizeof(glm::vec3)), &TempVertices[0],
                 GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    ElementCount = static_cast<int>(Indices.size());
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<long long>(ElementCount * sizeof(unsigned short)), &Indices[0],
                 GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void SCamera::Regenerate(float InFieldOfViewY, float InAspect) {
    FieldOfViewY = InFieldOfViewY;
    Aspect = InAspect;
    Projection = glm::perspective(glm::radians(FieldOfViewY), Aspect, 0.01f, 100.0f);
}

void SCamera::Update() {
    View = glm::lookAtRH(Position, Target, glm::vec3{0.0f, 1.0f, 0.0f});
}

void SFrameBuffer::Init(int TextureUnitID, int Width, int Height, glm::vec3 InClearColor) {
    ClearColor = InClearColor;

    glActiveTexture(GL_TEXTURE0 + TextureUnitID);
    glGenTextures(1, &ColorID);
    glBindTexture(GL_TEXTURE_2D, ColorID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glActiveTexture(GL_TEXTURE0 + TEXTURE_UNIT_TRANSIENT);

    glGenTextures(1, &DepthID);
    glBindTexture(GL_TEXTURE_2D, DepthID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, Width, Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, ColorID, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, DepthID, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void SFrameBuffer::Cleanup() {
    glDeleteFramebuffers(1, &FBO);
    glDeleteTextures(1, &ColorID);
    glDeleteTextures(1, &DepthID);

    std::cout << "Deleting SFrameBuffer..." << std::endl;
}

void SFrameBuffer::BindForDrawing() const {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);
    glClearColor(ClearColor.x, ClearColor.y, ClearColor.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void SFrameBuffer::BindForReading() const {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, FBO);
}

void SUniformBlock::Init(int Size) {
    glGenBuffers(1, &UBO);
    glBindBuffer(GL_UNIFORM_BUFFER, UBO);
    glBufferData(GL_UNIFORM_BUFFER, Size, nullptr, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void SUniformBlock::Cleanup() {
    glDeleteBuffers(1, &UBO);
}

void SUniformBlock::Bind() const {
//    glBindBufferRange(GL_UNIFORM_BUFFER, 0, UBO, 0, Size);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBO);
}

void SUniformBlock::SetMatrix(int Position, const glm::mat4x4 &Value) const {
    glBindBuffer(GL_UNIFORM_BUFFER, UBO);
    glBufferSubData(GL_UNIFORM_BUFFER, Position, sizeof(Value), &Value[0]);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void SUniformBlock::SetVector2(int Position, const glm::vec2 &Value) const {
    glBindBuffer(GL_UNIFORM_BUFFER, UBO);
    glBufferSubData(GL_UNIFORM_BUFFER, Position, sizeof(Value), &Value[0]);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void SUniformBlock::SetFloat(int Position, const float Value) const {
    glBindBuffer(GL_UNIFORM_BUFFER, UBO);
    glBufferSubData(GL_UNIFORM_BUFFER, Position, sizeof(Value), &Value);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void SRenderer::Init(int Width, int Height) {
    /** Common OpenGL settings */
    glDepthFunc(GL_LESS);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /** Setup a quad for 2D rendering */
    glGenVertexArrays(1, &Quad2D.VAO);
    glBindVertexArray(Quad2D.VAO);

    const float QuadVertices[] = {
            1.0f, 1.0f,
            1.0f, 0.0f,
            0.0f, 0.0f,
            0.0f, 1.0f,
    };
    glEnableVertexAttribArray(0);
    glGenBuffers(1, &Quad2D.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, Quad2D.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(QuadVertices), &QuadVertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    const float QuadUVs[] = {
            1.0f, 1.0f,
            1.0f, 0.0f,
            0.0f, 0.0f,
            0.0f, 1.0f
    };
    glEnableVertexAttribArray(1);
    glGenBuffers(1, &Quad2D.CBO);
    glBindBuffer(GL_ARRAY_BUFFER, Quad2D.CBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(QuadUVs), &QuadUVs[0], GL_STATIC_DRAW);
    glVertexAttribPointer(
            1,
            2,
            GL_FLOAT,
            GL_FALSE,
            0,
            nullptr
    );

    const unsigned short QuadIndices[] = {
            0, 1, 2, 0, 2, 3
    };
    glGenBuffers(1, &Quad2D.EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Quad2D.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(QuadIndices), &QuadIndices[0], GL_STATIC_DRAW);
    Quad2D.ElementCount = std::size(QuadIndices);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    /** Initialize queues */
    Queue2D.Init(32);
    Queue2D.CommonUniformBlock.SetVector2(0, {static_cast<float>(Width), static_cast<float>(Height)});
    Queue2D.CommonUniformBlock.SetFloat(12, Utility::GetRandomFloat());

    Queue3D.Init(sizeof(glm::mat4x4) * 2);

    /** Initialize framebuffers */
    MainFrameBuffer.Init(TEXTURE_UNIT_MAIN_FRAMEBUFFER, Width, Height, WINDOW_CLEAR_COLOR);

    /** Initialize atlases */
    Atlases[ATLAS_COMMON].Init(TEXTURE_UNIT_ATLAS_COMMON);
    Atlases[ATLAS_PRIMARY2D].Init(TEXTURE_UNIT_ATLAS_PRIMARY2D);
    Atlases[ATLAS_PRIMARY3D].Init(TEXTURE_UNIT_ATLAS_PRIMARY3D);

    /** Initialize shaders */
    ProgramHUD.Init(&ResourceHUD_vert, &ResourceHUD_frag);
    ProgramHUD.Use();
    glUniform1i(ProgramHUD.UniformCommonAtlasID, TEXTURE_UNIT_ATLAS_COMMON);

    ProgramUber2D.Init(&ResourceUber2D_vert, &ResourceUber2D_frag);
    ProgramUber2D.Use();
    glUniform1i(ProgramUber2D.UniformCommonAtlasID, TEXTURE_UNIT_ATLAS_COMMON);
    glUniform1i(ProgramUber2D.UniformPrimaryAtlasID, TEXTURE_UNIT_ATLAS_PRIMARY2D);

    ProgramUber3D.Init(&ResourceUber3D_vert, &ResourceUber3D_frag);
    ProgramUber3D.Use();
    glUniform1i(ProgramUber3D.UniformCommonAtlasID, TEXTURE_UNIT_ATLAS_COMMON);
    glUniform1i(ProgramUber3D.UniformPrimaryAtlasID, TEXTURE_UNIT_ATLAS_PRIMARY3D);

    ProgramPostProcess.Init(&ResourcePostProcess_vert, &ResourcePostProcess_frag);
    ProgramPostProcess.Use();
    glUniform1i(ProgramPostProcess.UniformColorTextureID, TEXTURE_UNIT_MAIN_FRAMEBUFFER);

    Tileset.InitPlaceholder();
}

void SRenderer::Cleanup() {
    MainFrameBuffer.Cleanup();
    for (auto &Atlas: Atlases) {
        Atlas.Cleanup();
    }
    Quad2D.Cleanup();
    Tileset.Cleanup();
    ProgramHUD.Cleanup();
    ProgramUber2D.Cleanup();
    ProgramUber3D.Cleanup();
    ProgramPostProcess.Cleanup();
    Queue2D.Cleanup();
    Queue3D.Cleanup();
}

void SRenderer::Flush(const SWindowData &WindowData) {
    /** Begin Draw */
    MainFrameBuffer.BindForDrawing();

    /** Draw 3D */
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glViewport(SCENE_OFFSET, SCENE_HEIGHT - SCENE_OFFSET, SCENE_WIDTH, SCENE_HEIGHT);

    Queue3D.CommonUniformBlock.Bind();

    ProgramUber3D.Use();

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    for (int Index = 0; Index < Queue3D.CurrentIndex; ++Index) {
        const auto &Entry = Queue3D.Entries[Index];
        if (Entry.Geometry == nullptr) {
            continue;
        }
        glBindVertexArray(Entry.Geometry->VAO);
        if (Entry.InstancedDrawCall != nullptr) {
            for (int DrawCallIndex = 0; DrawCallIndex < Entry.InstancedDrawCallCount; ++DrawCallIndex) {
                auto &DrawCall = *(Entry.InstancedDrawCall + DrawCallIndex);
                if (DrawCall.Count > 0) {
                    glUniformMatrix4fv(ProgramUber3D.UniformModelID, DrawCall.Count, GL_FALSE,
                                       &DrawCall.Transform[0][0][0]);
                    glDrawElementsInstanced(GL_TRIANGLES,
                                            DrawCall.SubGeometry->ElementCount,
                                            GL_UNSIGNED_SHORT,
                                            reinterpret_cast<void *>(DrawCall.SubGeometry->ElementOffset),
                                            DrawCall.Count);
                }
            }
        } else {
            glUniformMatrix4fv(ProgramUber3D.UniformModelID, 1, GL_FALSE, &Entry.Model[0][0]);
            glDrawElements(GL_TRIANGLES, Entry.Geometry->ElementCount, GL_UNSIGNED_SHORT, nullptr);
        }
    }
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    /** Draw 2D */
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    Queue2D.CommonUniformBlock.Bind();

    Queue2D.CommonUniformBlock.SetFloat(8, WindowData.Seconds);

    glBindVertexArray(Quad2D.VAO);

    for (int Index = 0; Index < Queue2D.CurrentIndex; ++Index) {
        auto const &Entry = Queue2D.Entries[Index];

        SProgram2D const *Program = nullptr;
        switch (Entry.Program2DType) {
            case EProgram2DType::HUD:
                Program = &ProgramHUD;
                break;
            case EProgram2DType::Uber2D:
                Program = &ProgramUber2D;
            default:
                break;
        }
        glUseProgram(Program->ID);

        glUniform2f(Program->UniformPositionScreenSpaceID, Entry.Position.x, Entry.Position.y);
        glUniform2f(Program->UniformSizeScreenSpaceID, Entry.SizePixels.x, Entry.SizePixels.y);
        glUniform4fv(Program->UniformUVRectID, 1, &Entry.UVRect[0]);

        SEntryMode Mode;
        if (Entry.Mode.has_value()) {
            Mode = Entry.Mode.value();
            glUniform4fv(Program->UniformModeControlAID, 1, &Mode.ControlA[0]);
            glUniform4fv(Program->UniformModeControlBID, 1, &Mode.ControlB[0]);
        }
        glUniform1i(Program->UniformModeID, Mode.ID);

        if (Entry.Program2DType == EProgram2DType::Uber2D) {
            if (Mode.ID == UBER2D_MODE_BACK_BLUR) {
                glDrawElementsInstanced(GL_TRIANGLES, Quad2D.ElementCount, GL_UNSIGNED_SHORT, nullptr,
                                        static_cast<int>(Mode.ControlA[0]));
                glUniform1i(Program->UniformModeID, 0);
            }
        }

        glDrawElements(GL_TRIANGLES, Quad2D.ElementCount, GL_UNSIGNED_SHORT, nullptr);
    }

    /** Display everything */
    glDisable(GL_BLEND);

    MainFrameBuffer.BindForReading();
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    glViewport(0, 0, WindowData.Width, WindowData.Height);
    glClearColor(0.0f, 0.125f, 0.125f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glViewport(WindowData.BlitX, WindowData.BlitY, WindowData.BlitWidth, WindowData.BlitHeight);

    ProgramPostProcess.Use();
    glDrawElements(GL_TRIANGLES, Quad2D.ElementCount, GL_UNSIGNED_SHORT, nullptr);

    Queue2D.Reset();
    Queue3D.Reset();
}

void SRenderer::UploadProjectionAndViewFromCamera(const SCamera &Camera) const {
    auto ProjectionMatrix = Camera.Projection;
    auto ViewMatrix = Camera.View;

    Queue3D.CommonUniformBlock.SetMatrix(0, ProjectionMatrix);
    Queue3D.CommonUniformBlock.SetMatrix(sizeof(glm::mat4x4), ViewMatrix);
}

void SRenderer::DrawHUD(glm::vec3 Position, glm::vec2 Size, int Mode) {
    SEntry2D Entry;
    Entry.Program2DType = EProgram2DType::HUD;
    Entry.Position = Position;
    Entry.SizePixels = Size;

    Entry.Mode = SEntryMode{.ID = Mode};

    Queue2D.Enqueue(Entry);
}

void SRenderer::Draw2D(glm::vec3 Position, const SSpriteHandle &SpriteHandle) {
    SEntry2D Entry;
    Entry.Program2DType = EProgram2DType::Uber2D;
    Entry.Position = Position;
    Entry.SizePixels = SpriteHandle.Sprite->SizePixels;
    Entry.UVRect = SpriteHandle.Sprite->UVRect;

    Queue2D.Enqueue(Entry);
}

void
SRenderer::Draw2DEx(glm::vec3 Position, const SSpriteHandle &SpriteHandle, int Mode, glm::vec4 ModeControlA) {
    SEntry2D Entry;
    Entry.Program2DType = EProgram2DType::Uber2D;
    Entry.Position = Position;
    Entry.SizePixels = SpriteHandle.Sprite->SizePixels;
    Entry.UVRect = SpriteHandle.Sprite->UVRect;

    Entry.Mode = SEntryMode{.ID = Mode, .ControlA = ModeControlA};

    Queue2D.Enqueue(Entry);
}

void
SRenderer::Draw2DEx(glm::vec3 Position, const SSpriteHandle &SpriteHandle, int Mode, glm::vec4 ModeControlA,
                    glm::vec4 ModeControlB) {
    SEntry2D Entry;
    Entry.Program2DType = EProgram2DType::Uber2D;
    Entry.Position = Position;
    Entry.SizePixels = SpriteHandle.Sprite->SizePixels;
    Entry.UVRect = SpriteHandle.Sprite->UVRect;

    Entry.Mode = SEntryMode{.ID = Mode, .ControlA = ModeControlA, .ControlB = ModeControlB};

    Queue2D.Enqueue(Entry);
}

void SRenderer::Draw2DHaze(glm::vec3 Position, const SSpriteHandle &SpriteHandle, float XIntensity, float YIntensity,
                           float Speed) {

    SEntry2D Entry;
    Entry.Program2DType = EProgram2DType::Uber2D;
    Entry.Position = Position;
    Entry.SizePixels = SpriteHandle.Sprite->SizePixels;
    Entry.UVRect = SpriteHandle.Sprite->UVRect;

    Entry.Mode = SEntryMode{.ID = UBER2D_MODE_HAZE, .ControlA = {XIntensity, YIntensity, Speed, 0.0f}};

    Queue2D.Enqueue(Entry);
}

void
SRenderer::Draw2DBackBlur(glm::vec3 Position, const SSpriteHandle &SpriteHandle, float Count, float Speed, float Step) {
    SEntry2D Entry;
    Entry.Program2DType = EProgram2DType::Uber2D;
    Entry.Position = Position;
    Entry.SizePixels = SpriteHandle.Sprite->SizePixels;
    Entry.UVRect = SpriteHandle.Sprite->UVRect;

    Entry.Mode = SEntryMode{.ID = UBER2D_MODE_BACK_BLUR, .ControlA = {Count, Speed, Step, 0.0f}};

    Queue2D.Enqueue(Entry);
}

void SRenderer::Draw2DGlow(glm::vec3 Position, const SSpriteHandle &SpriteHandle, glm::vec3 Color, float Intensity) {
    SEntry2D Entry;
    Entry.Program2DType = EProgram2DType::Uber2D;
    Entry.Position = Position;
    Entry.SizePixels = SpriteHandle.Sprite->SizePixels;
    Entry.UVRect = SpriteHandle.Sprite->UVRect;

    Entry.Mode = SEntryMode{.ID = UBER2D_MODE_GLOW, .ControlA = {Color, Intensity}};

    Queue2D.Enqueue(Entry);
}

void
SRenderer::Draw2DDisintegrate(glm::vec3 Position, const SSpriteHandle &SpriteHandle, const SSpriteHandle &NoiseHandle,
                              float Progress) {
    SEntry2D Entry;
    Entry.Program2DType = EProgram2DType::Uber2D;
    Entry.Position = Position;
    Entry.SizePixels = SpriteHandle.Sprite->SizePixels;
    Entry.UVRect = SpriteHandle.Sprite->UVRect;

    Entry.Mode = SEntryMode{
            .ID = UBER2D_MODE_DISINTEGRATE,
            .ControlA = {Progress, 0.0f, 0.0f, 0.0f},
            .ControlB = {
                    NoiseHandle.Sprite->UVRect
            }
    };

    Queue2D.Enqueue(Entry);
}

void SRenderer::Draw3D(glm::vec3 Position, SGeometry *Geometry) {
    SEntry3D Entry;

    Entry.Geometry = Geometry;
    Entry.Model = glm::translate(glm::identity<glm::mat4x4>(), Position);

    Entry.Mode = SEntryMode{
            .ID = UBER3D_MODE_BASIC
    };

    Queue3D.Enqueue(Entry);
}

void SRenderer::Draw3DLevel(const SLevel &Level) {
    for (auto &DrawCall: LevelDrawData.DrawCalls) {
        DrawCall.Count = 0;
    }

    auto &FloorDrawCall = LevelDrawData.DrawCalls[ETileGeometryType::Floor];
    FloorDrawCall.SubGeometry = &Tileset.TileGeometry[ETileGeometryType::Floor];

    auto &WallDrawCall = LevelDrawData.DrawCalls[ETileGeometryType::Wall];
    WallDrawCall.SubGeometry = &Tileset.TileGeometry[ETileGeometryType::Wall];

    for (unsigned int X = 0; X < Level.Width; ++X) {
        for (unsigned int Y = 0; Y < Level.Height; ++Y) {
            const auto Index = (Y * Level.Width) + X;
            const auto &Tile = Level.Grid[Index];

            auto XOffset = static_cast<float>(X);
            auto YOffset = static_cast<float>(Y);

            if (Tile.Type == ETileType::Floor) {
                FloorDrawCall.Transform[FloorDrawCall.Count] =
                        glm::translate(glm::identity<glm::mat4x4>(), glm::vec3(XOffset, 0.0f, YOffset));
                FloorDrawCall.Count++;
            }

            for (unsigned Direction = 0; Direction < DIRECTION_COUNT; ++Direction) {
                auto &TileEdge = Tile.Edges[Direction];

                if (TileEdge != ETileEdgeType::Empty) {
                    auto Transform = glm::identity<glm::mat4x4>();
                    Transform = glm::translate(Transform, glm::vec3(XOffset, 0.0f, YOffset));
                    Transform = glm::rotate(Transform, Utility::RotationFromDirection(EDirection{Direction}),
                                            {0.0f, 1.0f, 0.0f});
                    WallDrawCall.Transform[WallDrawCall.Count] = Transform;
                    WallDrawCall.Count++;
                }
            }
        }
    }

    SEntry3D Entry;

    Entry.Geometry = &Tileset;
    Entry.Model = glm::translate(glm::identity<glm::mat4x4>(), glm::zero<glm::vec3>());
    Entry.InstancedDrawCall = &LevelDrawData.DrawCalls[0];
    Entry.InstancedDrawCallCount = 2;

    Entry.Mode = SEntryMode{
            .ID = UBER3D_MODE_LEVEL
    };

    Queue3D.Enqueue(Entry);
}

void STexture::InitFromPixels(int Width, int Height, bool bAlpha, const void *Pixels) {
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);
    glTexImage2D(GL_TEXTURE_2D, 0, bAlpha ? GL_RGBA : GL_RGB, Width, Height, 0, bAlpha ? GL_RGBA : GL_RGB,
                 GL_UNSIGNED_BYTE, Pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void STexture::InitEmpty(int Width, int Height, bool bAlpha) {
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);
    glTexImage2D(GL_TEXTURE_2D, 0, bAlpha ? GL_RGBA : GL_RGB, Width, Height, 0, bAlpha ? GL_RGBA : GL_RGB,
                 GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void STexture::InitFromResource(const SResource *Resource) {
    int Width, Height, Channels;
    auto const Image = stbi_load_from_memory(Resource->Ptr, static_cast<int>(Resource->Length), &Width,
                                             &Height, &Channels,
                                             4);
    InitFromPixels(Width, Height, Channels == 4, Image);
    stbi_image_free(Image);
}

void STexture::Cleanup() {
    glDeleteTextures(1, &ID);

    std::cout << "Deleting STexture..." << std::endl;
}

void STexture::BindToTextureUnit(int TextureUnit) const {
    glActiveTexture(GL_TEXTURE0 + TextureUnit);
    glBindTexture(GL_TEXTURE_2D, ID);
}

std::array<int, ATLAS_MAX_SPRITE_COUNT> SAtlas::SortingIndices{};

void SAtlas::Init(int InTextureUnitID) {
    TextureUnitID = InTextureUnitID;
    InitEmpty(WidthAndHeight, WidthAndHeight, true);
    std::iota(SortingIndices.begin(), SortingIndices.end(), 0);
}

SSpriteHandle SAtlas::AddSprite(const SResource *Resource) {
    int Width, Height, Channels, Result;
    Result = stbi_info_from_memory(Resource->Ptr, static_cast<int>(Resource->Length), &Width, &Height, &Channels);
    if (!Result) {
        abort();
    }
    Sprites[CurrentIndex].SizePixels = {Width, Height};
    Sprites[CurrentIndex].Resource = Resource;
    return {this, &Sprites[CurrentIndex++]};
}

void SAtlas::Build() {
    BindToTextureUnit(TextureUnitID);

    std::sort(SortingIndices.begin(), SortingIndices.end(), [&](const int &IndexA, const int &IndexB) {
        return Sprites[IndexA].SizePixels.y > Sprites[IndexB].SizePixels.y;
    });

    int CursorX{};
    int CursorY{};
    int MaxHeight{};

    for (int Index = 0; Index < CurrentIndex; ++Index) {
        auto &Sprite = Sprites[SortingIndices[Index]];
        int Width, Height, Channels;
        const auto Image = stbi_load_from_memory(Sprite.Resource->Ptr, static_cast<int>(Sprite.Resource->Length),
                                                 &Width,
                                                 &Height, &Channels,
                                                 4);

        if (CursorX + Width > WidthAndHeight) {
            CursorY += MaxHeight;
            CursorX = 0;
            MaxHeight = 0;
        }

        if (CursorY + Height > WidthAndHeight) {
            break;
        }

        float MinU = static_cast<float>(CursorX) / static_cast<float>(WidthAndHeight);
        float MinV = static_cast<float>(CursorY) / static_cast<float>(WidthAndHeight);
        float MaxU = MinU + (static_cast<float>(Width) / static_cast<float>(WidthAndHeight));
        float MaxV = MinV + (static_cast<float>(Height) / static_cast<float>(WidthAndHeight));
        Sprite.UVRect = {MinU, MinV, MaxU, MaxV};

        glTexSubImage2D(GL_TEXTURE_2D, 0,
                        CursorX, CursorY,
                        Width, Height,
                        GL_RGBA,
                        GL_UNSIGNED_BYTE, Image);

        stbi_image_free(Image);

        CursorX += Width;

        if (Height > MaxHeight) {
            MaxHeight = Height;
        }
    }
}

