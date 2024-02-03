#include "Draw.hxx"

#include <algorithm>
#include <numeric>
#include "CommonTypes.hxx"
#include "Log.hxx"
#include "Tile.hxx"
#include "glad/gl.h"
#include "Level.hxx"
#include "Constants.hxx"
#include "Utility.hxx"
#include "Math.hxx"
#include "Memory.hxx"

#define SIZE_OF_VECTOR_ELEMENT(Vector) ((GLsizeiptr)sizeof(decltype(Vector)::value_type))

static std::string const SharedConstants{
#define SHARED_CONSTANTS_LITERAL
#include "SharedConstants.hxx"
#undef SHARED_CONSTANTS_LITERAL
};

namespace Asset::Shader
{
    EXTERN_ASSET(SharedGLSL)
    EXTERN_ASSET(HUDVERT)
    EXTERN_ASSET(HUDFRAG)
    EXTERN_ASSET(MapVERT)
    EXTERN_ASSET(MapFRAG)
    EXTERN_ASSET(Uber2DVERT)
    EXTERN_ASSET(Uber2DFRAG)
    EXTERN_ASSET(Uber3DVERT)
    EXTERN_ASSET(Uber3DFRAG)
    EXTERN_ASSET(PostProcessVERT)
    EXTERN_ASSET(PostProcessFRAG)
}

struct SShaderMapData
{
    int32_t Width{};
    int32_t Height{};
    float POVX{};
    float POVY{};
    std::array<STile, MAX_LEVEL_TILE_COUNT> Tiles;
};

void SProgram::CheckShader(unsigned int ShaderID)
{
    int Success;
    glGetShaderiv(ShaderID, GL_COMPILE_STATUS, &Success);
    if (!Success)
    {
        GLint MaxLength = 0;
        glGetShaderiv(ShaderID, GL_INFO_LOG_LENGTH, &MaxLength);
        GLsizei LengthQuery(0);
        std::vector<GLchar> InfoLog(MaxLength + 1, '\0');
        glGetShaderInfoLog(ShaderID, GLsizei(InfoLog.size()), &LengthQuery, &InfoLog[0]);
        auto Error = std::string(InfoLog.begin(), InfoLog.end());
        Log::Draw<ELogLevel::Critical>("Shader error!\n %s", Error.c_str());
    }
}

void SProgram::CheckProgram(unsigned int ProgramID)
{
    int Success;
    glValidateProgram(ProgramID);
    glGetProgramiv(ProgramID, GL_VALIDATE_STATUS, &Success);
    if (!Success)
    {
        GLint MaxLength = 0;
        glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &MaxLength);
        GLsizei LengthQuery(0);
        std::vector<GLchar> InfoLog(MaxLength + 1, '\0');
        glGetProgramInfoLog(ProgramID, GLsizei(InfoLog.size()), &LengthQuery, &InfoLog[0]);
        auto Error = std::string(InfoLog.begin(), InfoLog.end());
        Log::Draw<ELogLevel::Critical>("Program error!\n %s", Error.c_str());
    }
}

unsigned int SProgram::CreateVertexShader(const char* Data, int Length)
{
    unsigned VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    char const* Blocks[4] = {
        &GLSLVersion[0],
        Asset::Shader::SharedGLSL.SignedCharPtr(),
        &SharedConstants[0],
        Data
    };
    int const Lengths[4] = {
        (int)GLSLVersion.length(),
        (int)Asset::Shader::SharedGLSL.Length,
        (int)SharedConstants.length(),
        Length
    };
    glShaderSource(VertexShaderID, 4, Blocks, &Lengths[0]);
    glCompileShader(VertexShaderID);
    CheckShader(VertexShaderID);
    return VertexShaderID;
}

unsigned int SProgram::CreateFragmentShader(const char* Data, int Length)
{
    unsigned FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    char const* Blocks[4] = {
        &GLSLVersion[0],
        Asset::Shader::SharedGLSL.SignedCharPtr(),
        &SharedConstants[0],
        Data
    };
    int const Lengths[4] = {
        (int)GLSLVersion.length(),
        (int)Asset::Shader::SharedGLSL.Length,
        (int)SharedConstants.length(),
        Length
    };
    glShaderSource(FragmentShaderID, 4, Blocks, &Lengths[0]);
    glCompileShader(FragmentShaderID);
    CheckShader(FragmentShaderID);
    return FragmentShaderID;
}

unsigned int SProgram::CreateProgram(unsigned int VertexShader, unsigned int FragmentShader)
{
    unsigned ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShader);
    glAttachShader(ProgramID, FragmentShader);
    glLinkProgram(ProgramID);
    CheckProgram(ProgramID);
    return ProgramID;
}

void SProgram::InitUniforms()
{
    UniformModeID = glGetUniformLocation(ID, "u_mode");
    UniformModeControlAID = glGetUniformLocation(ID, "u_modeControlA");
    UniformModeControlBID = glGetUniformLocation(ID, "u_modeControlB");
}

void SProgram::Init(const SAsset& InVertexShaderAsset, const SAsset& InFragmentShaderAsset)
{
#ifdef EQUINOX_REACH_DEVELOPMENT
    VertexShaderAsset = &InVertexShaderAsset;
    FragmentShaderAsset = &InFragmentShaderAsset;
#endif
    unsigned VertexShader = CreateVertexShader(InVertexShaderAsset.SignedCharPtr(), (int)InVertexShaderAsset.Length);
    unsigned FragmentShader = CreateFragmentShader(InFragmentShaderAsset.SignedCharPtr(), (int)InFragmentShaderAsset.Length);
    ID = CreateProgram(VertexShader, FragmentShader);
    glDeleteShader(VertexShader);
    glDeleteShader(FragmentShader);
    SProgram::InitUniforms();
    InitUniforms();
}

void SProgram::Cleanup() const
{
    glDeleteProgram(ID);

    Log::Draw<ELogLevel::Debug>("Deleting SProgram");
}

void SProgram::Use() const
{
    glUseProgram(ID);
}

#ifdef EQUINOX_REACH_DEVELOPMENT
    #include <sstream>
void SProgram::Reload()
{
    namespace fs = std::filesystem;

    std::ifstream ShaderFile;

    unsigned VertexShader, FragmentShader;

    {
        ShaderFile.open(VertexShaderAsset->Path(), std::ifstream::in);
        std::stringstream VertexShaderStringBuffer;
        VertexShaderStringBuffer << ShaderFile.rdbuf();
        auto VertexShaderString = VertexShaderStringBuffer.str();
        VertexShader = CreateVertexShader(VertexShaderString.data(), (int)VertexShaderString.length());

        ShaderFile.close();
    }

    {
        ShaderFile.open(FragmentShaderAsset->Path(), std::ifstream::in);
        std::stringstream FragmentShaderStringBuffer;
        FragmentShaderStringBuffer << ShaderFile.rdbuf();
        auto FragmentShaderString = FragmentShaderStringBuffer.str();
        FragmentShader = CreateFragmentShader(FragmentShaderString.data(), (int)FragmentShaderString.length());

        ShaderFile.close();
    }

    Cleanup();
    ID = CreateProgram(VertexShader, FragmentShader);
    glDeleteShader(VertexShader);
    glDeleteShader(FragmentShader);
    SProgram::InitUniforms();
    InitUniforms();

    Log::Draw<ELogLevel::Debug>("Reloading SProgram");
}
#endif

void SProgramPostProcess::InitUniforms()
{
    UniformColorTextureID = glGetUniformLocation(ID, "u_colorTexture");
}

void SProgram3D::InitUniforms()
{
    UniformBlockCommon3D = glGetUniformBlockIndex(ID, "ub_common");
    glUniformBlockBinding(ID, UniformBlockCommon3D, 0);
    UniformModelID = glGetUniformLocation(ID, "u_model");
    UniformCommonAtlasID = glGetUniformLocation(ID, "u_commonAtlas");
    UniformPrimaryAtlasID = glGetUniformLocation(ID, "u_primaryAtlas");
}

void SProgram2D::InitUniforms()
{
    UniformBlockCommon2D = glGetUniformBlockIndex(ID, "ub_common");
    glUniformBlockBinding(ID, UniformBlockCommon2D, 0);
    UniformPositionScreenSpaceID = glGetUniformLocation(ID, "u_positionScreenSpace");
    UniformSizeScreenSpaceID = glGetUniformLocation(ID, "u_sizeScreenSpace");
    UniformUVRectID = glGetUniformLocation(ID, "u_uvRect");
    UniformCommonAtlasID = glGetUniformLocation(ID, "u_commonAtlas");
    UniformPrimaryAtlasID = glGetUniformLocation(ID, "u_primaryAtlas");
}

void SProgramHUD::InitUniforms()
{
    SProgram2D::InitUniforms();

    UniformMap = glGetUniformBlockIndex(ID, "ub_map");
    glUniformBlockBinding(ID, UniformMap, 1);
}

void SProgramMap::InitUniforms()
{
    UniformBlockCommon2D = glGetUniformBlockIndex(ID, "ub_common");
    glUniformBlockBinding(ID, UniformBlockCommon2D, 0);

    UniformMap = glGetUniformBlockIndex(ID, "ub_map");
    glUniformBlockBinding(ID, UniformMap, 1);

    UniformPositionScreenSpaceID = glGetUniformLocation(ID, "u_positionScreenSpace");
    UniformSizeScreenSpaceID = glGetUniformLocation(ID, "u_sizeScreenSpace");
}

void SGeometry::InitFromRawMesh(const CRawMesh& RawMesh)
{
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glEnableVertexAttribArray(0);
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
        (GLsizeiptr)RawMesh.Positions.size() * SIZE_OF_VECTOR_ELEMENT(RawMesh.Positions),
        &RawMesh.Positions[0],
        GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glEnableVertexAttribArray(1);
    glGenBuffers(1, &CBO);
    glBindBuffer(GL_ARRAY_BUFFER, CBO);
    glBufferData(GL_ARRAY_BUFFER,
        (GLsizeiptr)RawMesh.TexCoords.size() * SIZE_OF_VECTOR_ELEMENT(RawMesh.TexCoords),
        &RawMesh.TexCoords[0], GL_STATIC_DRAW);
    glVertexAttribPointer(
        1,
        2,
        GL_FLOAT,
        GL_FALSE,
        0,
        nullptr);

    ElementCount = (int)RawMesh.Indices.size();
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        ElementCount * SIZE_OF_VECTOR_ELEMENT(RawMesh.Indices), &RawMesh.Indices[0],
        GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void SGeometry::Cleanup()
{
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &CBO);
    glDeleteVertexArrays(1, &VAO);

    Log::Draw<ELogLevel::Debug>("Deleting SGeometry with ElementCount: %d", ElementCount);
}

void STileset::InitPlaceholder()
{
    std::array<UVec3, 8> TempVertices{};
    std::array<unsigned short, 12> Indices{};

    /** Floor Quad */
    auto& FloorGeometry = TileGeometry[ETileGeometryType::Floor];
    FloorGeometry.ElementOffset = 0;
    FloorGeometry.ElementCount = 6;
    Indices[0] = 0;
    Indices[1] = 1;
    Indices[2] = 2;
    Indices[3] = 0;
    Indices[4] = 2;
    Indices[5] = 3;

    TempVertices[0] = { 0.5f, 0.0f, 0.5f };
    TempVertices[1] = { 0.5f, 0.0f, -0.5f };
    TempVertices[2] = { -0.5f, 0.0f, -0.5f };
    TempVertices[3] = { -0.5f, 0.0f, 0.5f };

    /** Wall Quad */
    auto& WallGeometry = TileGeometry[ETileGeometryType::Wall];
    WallGeometry.ElementOffset = 12;
    WallGeometry.ElementCount = 6;
    Indices[6] = 4 + 0;
    Indices[7] = 4 + 1;
    Indices[8] = 4 + 2;
    Indices[9] = 4 + 0;
    Indices[10] = 4 + 2;
    Indices[11] = 4 + 3;

    TempVertices[4] = { 0.5f, 1.0f, -0.5f };
    TempVertices[5] = { -0.5f, 1.0f, -0.5f };
    TempVertices[6] = { -0.5f, 0.0f, -0.5f };
    TempVertices[7] = { 0.5f, 0.0f, -0.5f };

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, TempVertices.size() * (GLsizeiptr)sizeof(UVec3), &TempVertices[0],
        GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    ElementCount = (int)Indices.size();
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ElementCount * (GLsizeiptr)sizeof(unsigned short), &Indices[0],
        GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void STileset::InitBasic(
    const SAsset& Floor,
    const SAsset& Hole,
    const SAsset& Wall,
    const SAsset& WallJoint,
    const SAsset& DoorFrame,
    const SAsset& Door)
{
    auto Positions = Memory::GetVector<UVec3>();
    auto TexCoords = Memory::GetVector<UVec2>();
    auto Indices = Memory::GetVector<unsigned short>();

    int LastElementOffset = 0;
    int LastVertexOffset = 0;

    auto InitGeometry = [&](const SAsset& Resource, int Type) {
        auto Mesh = CRawMesh(Resource);

        auto& Geometry = TileGeometry[Type];
        Geometry.ElementOffset = LastElementOffset;
        Geometry.ElementCount = Mesh.GetElementCount();

        for (int Index = 0; Index < Geometry.ElementCount; ++Index)
        {
            Indices.push_back(Mesh.Indices[Index] + LastVertexOffset);
        }

        for (int Index = 0; Index < Mesh.GetVertexCount(); ++Index)
        {
            Positions.push_back(Mesh.Positions[Index]);
            TexCoords.push_back(Mesh.TexCoords[Index]);
        }

        LastVertexOffset += Mesh.GetVertexCount();
        LastElementOffset += (int)((Geometry.ElementCount) * sizeof(unsigned short));
    };

    InitGeometry(Floor, ETileGeometryType::Floor);
    InitGeometry(Hole, ETileGeometryType::Hole);
    InitGeometry(Wall, ETileGeometryType::Wall);
    InitGeometry(WallJoint, ETileGeometryType::WallJoint);
    InitGeometry(DoorFrame, ETileGeometryType::DoorFrame);
    InitGeometry(Door, ETileGeometryType::Door);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glEnableVertexAttribArray(0);
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)Positions.size() * (GLsizeiptr)sizeof(UVec3), &Positions[0],
        GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glEnableVertexAttribArray(1);
    glGenBuffers(1, &CBO);
    glBindBuffer(GL_ARRAY_BUFFER, CBO);
    glBufferData(GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(TexCoords.size() * SIZE_OF_VECTOR_ELEMENT(TexCoords)),
        &TexCoords[0], GL_STATIC_DRAW);
    glVertexAttribPointer(
        1,
        2,
        GL_FLOAT,
        GL_FALSE,
        0,
        nullptr);

    ElementCount = (int)Indices.size();
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ElementCount * (GLsizeiptr)sizeof(unsigned short), &Indices[0],
        GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void SCamera::RegenerateProjection()
{
    float const FOVRadians = Math::Radians(FieldOfViewY);
    float const TanHalfFovY = std::tan(FOVRadians / 2.0f);

    Projection = UMat4x4();
    Projection.X.X = 1.0f / (Aspect * TanHalfFovY);
    Projection.Y.Y = 1.0f / (TanHalfFovY);
    Projection.Z.Z = -(ZFar + ZNear) / (ZFar - ZNear);
    Projection.Z.W = -1.0f;
    Projection.W.Z = -(2.0f * ZFar * ZNear) / (ZFar - ZNear);
}

void SCamera::Update()
{
    View = UMat4x4::LookAtRH(Position, Target, UVec3{ 0.0f, 1.0f, 0.0f });
}

void SFrameBuffer::Init(int TextureUnitID, int InWidth, int InHeight, UVec3 InClearColor, bool bLinearFiltering)
{
    Width = InWidth;
    Height = InHeight;
    ClearColor = InClearColor;

    glActiveTexture(GL_TEXTURE0 + TextureUnitID);
    glGenTextures(1, &ColorID);
    glBindTexture(GL_TEXTURE_2D, ColorID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, InWidth, InHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    if (bLinearFiltering)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    /* @TODO: Why is it transient? */
    glActiveTexture(GL_TEXTURE0 + TEXTURE_UNIT_TRANSIENT);

    glGenTextures(1, &DepthID);
    glBindTexture(GL_TEXTURE_2D, DepthID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, InWidth, InHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, ColorID, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, DepthID, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void SFrameBuffer::Cleanup()
{
    glDeleteFramebuffers(1, &FBO);
    glDeleteTextures(1, &ColorID);
    glDeleteTextures(1, &DepthID);

    Log::Draw<ELogLevel::Debug>("Deleting SFrameBuffer");
}

void SFrameBuffer::BindForDrawing() const
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);
    glClearColor(ClearColor.X, ClearColor.Y, ClearColor.Z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void SFrameBuffer::BindForReading() const
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, FBO);
}

void SFrameBuffer::Unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SUniformBlock::Init(int Size)
{
    glGenBuffers(1, &UBO);
    glBindBuffer(GL_UNIFORM_BUFFER, UBO);
    glBufferData(GL_UNIFORM_BUFFER, Size, nullptr, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void SUniformBlock::Cleanup()
{
    glDeleteBuffers(1, &UBO);
}

void SUniformBlock::Bind(int BindingPoint) const
{
    // glBindBufferRange(GL_UNIFORM_BUFFER, 0, UBO, 0, Size);
    glBindBufferBase(GL_UNIFORM_BUFFER, BindingPoint, UBO);
}

void SUniformBlock::SetMatrix(int Position, const UMat4x4& Value) const
{
    glBindBuffer(GL_UNIFORM_BUFFER, UBO);
    glBufferSubData(GL_UNIFORM_BUFFER, Position, sizeof(Value), &Value.X);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void SUniformBlock::SetVector2(int Position, const UVec2& Value) const
{
    glBindBuffer(GL_UNIFORM_BUFFER, UBO);
    glBufferSubData(GL_UNIFORM_BUFFER, Position, sizeof(Value), &Value.X);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void SUniformBlock::SetFloat(int Position, const float Value) const
{
    glBindBuffer(GL_UNIFORM_BUFFER, UBO);
    glBufferSubData(GL_UNIFORM_BUFFER, Position, sizeof(Value), &Value);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void SRenderer::Init(int Width, int Height)
{
    /** Common OpenGL settings */
    glDepthFunc(GL_LESS);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /** Setup a quad for 2D rendering */
    glGenVertexArrays(1, &Quad2D.VAO);
    glBindVertexArray(Quad2D.VAO);

    const float QuadVertices[] = {
        1.0f,
        1.0f,
        1.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        1.0f,
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
        nullptr);

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
    Queue2D.CommonUniformBlock.SetVector2(0, { (float)Width, (float)Height });
    Queue2D.CommonUniformBlock.SetFloat(12, Utility::GetRandomFloat());

    Queue3D.Init(sizeof(UMat4x4) * 2);

    /** Initialize framebuffers */
    MainFrameBuffer.Init(TEXTURE_UNIT_MAIN_FRAMEBUFFER, Width, Height, WINDOW_CLEAR_COLOR);

    /** Initialize atlases */
    Atlases[ATLAS_COMMON].Init(TEXTURE_UNIT_ATLAS_COMMON);
    Atlases[ATLAS_PRIMARY2D].Init(TEXTURE_UNIT_ATLAS_PRIMARY2D);
    Atlases[ATLAS_PRIMARY3D].Init(TEXTURE_UNIT_ATLAS_PRIMARY3D);

    /** Initialize shaders */
    MapUniformBlock.Init(16 * MAX_LEVEL_TILE_COUNT + 16);

    ProgramHUD.Init(Asset::Shader::HUDVERT, Asset::Shader::HUDFRAG);
    ProgramHUD.Use();
    glUniform1i(ProgramHUD.UniformCommonAtlasID, TEXTURE_UNIT_ATLAS_COMMON);

    ProgramMap.Init(Asset::Shader::MapVERT, Asset::Shader::MapFRAG);
    ProgramMap.Use();
    glUniform1i(ProgramHUD.UniformCommonAtlasID, TEXTURE_UNIT_ATLAS_COMMON);

    ProgramUber2D.Init(Asset::Shader::Uber2DVERT, Asset::Shader::Uber2DFRAG);
    ProgramUber2D.Use();
    glUniform1i(ProgramUber2D.UniformCommonAtlasID, TEXTURE_UNIT_ATLAS_COMMON);
    glUniform1i(ProgramUber2D.UniformPrimaryAtlasID, TEXTURE_UNIT_ATLAS_PRIMARY2D);

    ProgramUber3D.Init(Asset::Shader::Uber3DVERT, Asset::Shader::Uber3DFRAG);
    ProgramUber3D.Use();
    glUniform1i(ProgramUber3D.UniformCommonAtlasID, TEXTURE_UNIT_ATLAS_COMMON);
    glUniform1i(ProgramUber3D.UniformPrimaryAtlasID, TEXTURE_UNIT_ATLAS_PRIMARY3D);

    ProgramPostProcess.Init(Asset::Shader::PostProcessVERT, Asset::Shader::PostProcessFRAG);
    ProgramPostProcess.Use();
    glUniform1i(ProgramPostProcess.UniformColorTextureID, TEXTURE_UNIT_MAIN_FRAMEBUFFER);
}

void SRenderer::Cleanup()
{
    MainFrameBuffer.Cleanup();
    for (auto& Atlas : Atlases)
    {
        Atlas.Cleanup();
    }
    Quad2D.Cleanup();
    MapUniformBlock.Cleanup();
    ProgramHUD.Cleanup();
    ProgramMap.Cleanup();
    ProgramUber2D.Cleanup();
    ProgramUber3D.Cleanup();
    ProgramPostProcess.Cleanup();
    Queue2D.Cleanup();
    Queue3D.Cleanup();
}

void SRenderer::SetupLevelDrawData(const STileset& TileSet)
{
    LevelDrawData.TileSet = &TileSet;
    for (int TileTypeIndex = 0; TileTypeIndex < ETileGeometryType::Count; TileTypeIndex++)
    {
        auto& DrawCall = LevelDrawData.DrawCalls[TileTypeIndex];
        DrawCall.SubGeometry = &TileSet.TileGeometry[TileTypeIndex];
    }
}

void SRenderer::UploadLevelMapData(const SLevel& Level) const
{
    SShaderMapData ShaderMapData{};

    ShaderMapData.Width = (int)Level.Width;
    ShaderMapData.Height = (int)Level.Height;

    ShaderMapData.Tiles = Level.Tiles;

    glBindBuffer(GL_UNIFORM_BUFFER, MapUniformBlock.UBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(SShaderMapData), &ShaderMapData);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void SRenderer::Flush(const SWindowData& WindowData)
{
    /* Begin Draw */
    MainFrameBuffer.BindForDrawing();

    /* Draw 3D */
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glViewport(SCENE_OFFSET, SCENE_OFFSET, SCENE_WIDTH, SCENE_HEIGHT);

    Queue3D.CommonUniformBlock.Bind(0);

    ProgramUber3D.Use();

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    for (int Index = 0; Index < Queue3D.CurrentIndex; ++Index)
    {
        const auto& Entry = Queue3D.Entries[Index];
        if (Entry.Geometry == nullptr)
        {
            continue;
        }
        glUniform1i(ProgramUber3D.UniformModeID, Entry.Mode.ID);
        glBindVertexArray(Entry.Geometry->VAO);
        if (Entry.InstancedDrawCall != nullptr)
        {
            for (int DrawCallIndex = 0; DrawCallIndex < Entry.InstancedDrawCallCount; ++DrawCallIndex)
            {
                auto& DrawCall = *(Entry.InstancedDrawCall + DrawCallIndex);
                auto TotalCount = DrawCall.Count + DrawCall.DynamicCount;
                if (TotalCount > 0)
                {
                    glUniformMatrix4fv(ProgramUber3D.UniformModelID, TotalCount, GL_FALSE,
                        &DrawCall.Transform[0].X.X);
                    glDrawElementsInstanced(GL_TRIANGLES,
                        DrawCall.SubGeometry->ElementCount,
                        GL_UNSIGNED_SHORT,
                        reinterpret_cast<void*>(DrawCall.SubGeometry->ElementOffset),
                        TotalCount);
                }
            }
        }
        else
        {
            glUniformMatrix4fv(ProgramUber3D.UniformModelID, 1, GL_FALSE, &Entry.Model.X.X);
            glDrawElements(GL_TRIANGLES, Entry.Geometry->ElementCount, GL_UNSIGNED_SHORT, nullptr);
        }
    }
    // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    /* Draw 2D */
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    Queue2D.CommonUniformBlock.Bind(0);
    MapUniformBlock.Bind(1);

    Queue2D.CommonUniformBlock.SetVector2(0, { SCREEN_WIDTH, SCREEN_HEIGHT });
    Queue2D.CommonUniformBlock.SetFloat(8, WindowData.Seconds);

    glBindVertexArray(Quad2D.VAO);

    for (int Index = 0; Index < Queue2D.CurrentIndex; ++Index)
    {
        auto const& Entry = Queue2D.Entries[Index];

        SProgram2D const* Program;
        switch (Entry.Program2DType)
        {
            case EProgram2DType::HUD:
                Program = &ProgramHUD;
                break;
            case EProgram2DType::Uber2D:
                Program = &ProgramUber2D;
                break;
            default:
                continue;
        }
        Program->Use();

        glUniform2f(Program->UniformPositionScreenSpaceID, Entry.Position.X, Entry.Position.Y);
        glUniform2f(Program->UniformSizeScreenSpaceID, (float)Entry.SizePixels.X, (float)Entry.SizePixels.Y);
        glUniform4fv(Program->UniformUVRectID, 1, &Entry.UVRect.X);

        const SEntryMode& Mode = Entry.Mode;
        if (Mode.ID > UBER2D_MODE_TEXTURE)
        {
            glUniform4fv(Program->UniformModeControlAID, 1, &Mode.ControlA.X);
            glUniform4fv(Program->UniformModeControlBID, 1, &Mode.ControlB.X);
        }
        glUniform1i(Program->UniformModeID, Mode.ID);

        if (Entry.Program2DType == EProgram2DType::Uber2D)
        {
            if (Mode.ID == UBER2D_MODE_BACK_BLUR)
            {
                glDrawElementsInstanced(GL_TRIANGLES, Quad2D.ElementCount, GL_UNSIGNED_SHORT, nullptr,
                    (int)Mode.ControlA.X);
                glUniform1i(Program->UniformModeID, 0);
            }
        }
        else if (Entry.Program2DType == EProgram2DType::HUD)
        {
            if (Mode.ID == HUD_MODE_MAP)
            {
            }
        }

        glDrawElements(GL_TRIANGLES, Quad2D.ElementCount, GL_UNSIGNED_SHORT, nullptr);
    }

    /* Display everything */
    glDisable(GL_BLEND);

    MainFrameBuffer.BindForReading();

    glViewport(0, 0, WindowData.Width, WindowData.Height);
    glClearColor(0.0f, 0.125f, 0.125f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glViewport(WindowData.BlitX, WindowData.BlitY, WindowData.BlitWidth, WindowData.BlitHeight);

    ProgramPostProcess.Use();
    glDrawElements(GL_TRIANGLES, Quad2D.ElementCount, GL_UNSIGNED_SHORT, nullptr);

    Queue2D.Reset();
    Queue3D.Reset();
}

void SRenderer::UploadProjectionAndViewFromCamera(const SCamera& Camera) const
{
    auto ProjectionMatrix = Camera.Projection;
    auto ViewMatrix = Camera.View;

    Queue3D.CommonUniformBlock.SetMatrix(0, ProjectionMatrix);
    Queue3D.CommonUniformBlock.SetMatrix(sizeof(UMat4x4), ViewMatrix);
}

void SRenderer::DrawHUD(UVec3 Position, UVec2Int Size, int Mode)
{
    SEntry2D Entry;
    Entry.Program2DType = EProgram2DType::HUD;
    Entry.Position = Position;
    Entry.SizePixels = Size;

    Entry.Mode = SEntryMode{ Mode };

    Queue2D.Enqueue(Entry);
}

void SRenderer::DrawHUDMap(SLevel& Level, UVec3 Position, UVec2Int Size, const UVec2& POVOrigin)
{
    glBindBuffer(GL_UNIFORM_BUFFER, MapUniformBlock.UBO);
    if (Level.DirtyFlags & ELevelDirtyFlags::POVChanged)
    {
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(SShaderMapData, POVX), sizeof(POVOrigin), &POVOrigin);

        Level.DirtyFlags &= ~ELevelDirtyFlags::POVChanged;

        Log::Draw<ELogLevel::Verbose>("%s(): POVOrigin: { %.2f, %.2f }", __func__, POVOrigin.X, POVOrigin.Y);
    }

    if (Level.DirtyFlags & ELevelDirtyFlags::DirtyRange)
    {
        auto DirtyCount = (GLsizeiptr)(Level.DirtyRange.Y - Level.DirtyRange.X) + 1;
        auto FirstTile = Level.GetTile(Level.DirtyRange.X);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(SShaderMapData, Tiles) + (Level.DirtyRange.X * sizeof(STile)), DirtyCount * (GLsizeiptr)sizeof(STile), FirstTile);

        Level.DirtyFlags &= ~ELevelDirtyFlags::DirtyRange;

        Log::Draw<ELogLevel::Debug>("%s(): DirtyRange: %d to %d", __func__, Level.DirtyRange.X, Level.DirtyRange.Y);
    }
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    SEntry2D Entry;
    Entry.Program2DType = EProgram2DType::HUD;
    Entry.Position = Position;
    Entry.SizePixels = Size;

    Entry.Mode = SEntryMode{ HUD_MODE_MAP };

    Queue2D.Enqueue(Entry);
}

void SRenderer::DrawMapImmediate(SLevel& Level, UVec3 Position, UVec2Int Size, float Time)
{
    UVec2 POVOrigin{ (float)Level.Width / 2.0f, (float)Level.Height / 2.0f };
    int Count = Level.Width * Level.Height;

    ProgramMap.Use();

    Queue2D.CommonUniformBlock.Bind(0);
    MapUniformBlock.Bind(1);

    Queue2D.CommonUniformBlock.SetVector2(0, { (float)Size.X, (float)Size.Y });
    Queue2D.CommonUniformBlock.SetFloat(8, Time);

    glUniform1i(ProgramMap.UniformModeID, MAP_MODE_EDITOR);
    glUniform2f(ProgramMap.UniformPositionScreenSpaceID, Position.X, Position.Y);
    glUniform2f(ProgramMap.UniformSizeScreenSpaceID, (float)Size.X, (float)Size.Y);

    /* @TODO: Update tiles every frame for now. */
    UploadLevelMapData(Level);
    glBindBuffer(GL_UNIFORM_BUFFER, MapUniformBlock.UBO);
    glBufferSubData(GL_UNIFORM_BUFFER, offsetof(SShaderMapData, POVX), sizeof(POVOrigin), &POVOrigin);
    // glBufferSubData(GL_UNIFORM_BUFFER, offsetof(SShaderMapData, POVX), sizeof(POVOrigin), &POVOrigin);
    // glBufferSubData(GL_UNIFORM_BUFFER, offsetof(SShaderMapData, Tiles), Count * (GLsizeiptr)sizeof(STile), &Level.Tiles[0]);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindVertexArray(Quad2D.VAO);

    glDrawElements(GL_TRIANGLES, Quad2D.ElementCount, GL_UNSIGNED_SHORT, nullptr);

    glBindVertexArray(0);
}

void SRenderer::Draw2D(UVec3 Position, const SSpriteHandle& SpriteHandle)
{
    SEntry2D Entry;
    Entry.Program2DType = EProgram2DType::Uber2D;
    Entry.Position = Position;
    Entry.SizePixels = SpriteHandle.Sprite->SizePixels;
    Entry.UVRect = SpriteHandle.Sprite->UVRect;

    Queue2D.Enqueue(Entry);
}

void SRenderer::Draw2DEx(UVec3 Position, const SSpriteHandle& SpriteHandle, int Mode, UVec4 ModeControlA)
{
    SEntry2D Entry;
    Entry.Program2DType = EProgram2DType::Uber2D;
    Entry.Position = Position;
    Entry.SizePixels = SpriteHandle.Sprite->SizePixels;
    Entry.UVRect = SpriteHandle.Sprite->UVRect;

    Entry.Mode = SEntryMode{ Mode, ModeControlA };

    Queue2D.Enqueue(Entry);
}

void SRenderer::Draw2DEx(UVec3 Position, const SSpriteHandle& SpriteHandle, int Mode, UVec4 ModeControlA,
    UVec4 ModeControlB)
{
    SEntry2D Entry;
    Entry.Program2DType = EProgram2DType::Uber2D;
    Entry.Position = Position;
    Entry.SizePixels = SpriteHandle.Sprite->SizePixels;
    Entry.UVRect = SpriteHandle.Sprite->UVRect;

    Entry.Mode = SEntryMode{ Mode, ModeControlA, ModeControlB };

    Queue2D.Enqueue(Entry);
}

void SRenderer::Draw2DHaze(UVec3 Position, const SSpriteHandle& SpriteHandle, float XIntensity, float YIntensity,
    float Speed)
{

    SEntry2D Entry;
    Entry.Program2DType = EProgram2DType::Uber2D;
    Entry.Position = Position;
    Entry.SizePixels = SpriteHandle.Sprite->SizePixels;
    Entry.UVRect = SpriteHandle.Sprite->UVRect;

    Entry.Mode = SEntryMode{ UBER2D_MODE_HAZE, { XIntensity, YIntensity, Speed, 0.0f } };

    Queue2D.Enqueue(Entry);
}

void SRenderer::Draw2DBackBlur(UVec3 Position, const SSpriteHandle& SpriteHandle, float Count, float Speed, float Step)
{
    SEntry2D Entry;
    Entry.Program2DType = EProgram2DType::Uber2D;
    Entry.Position = Position;
    Entry.SizePixels = SpriteHandle.Sprite->SizePixels;
    Entry.UVRect = SpriteHandle.Sprite->UVRect;

    Entry.Mode = SEntryMode{ UBER2D_MODE_BACK_BLUR, { Count, Speed, Step, 0.0f } };

    Queue2D.Enqueue(Entry);
}

void SRenderer::Draw2DGlow(UVec3 Position, const SSpriteHandle& SpriteHandle, UVec3 Color, float Intensity)
{
    SEntry2D Entry;
    Entry.Program2DType = EProgram2DType::Uber2D;
    Entry.Position = Position;
    Entry.SizePixels = SpriteHandle.Sprite->SizePixels;
    Entry.UVRect = SpriteHandle.Sprite->UVRect;

    Entry.Mode = SEntryMode{ UBER2D_MODE_GLOW, { Color, Intensity } };

    Queue2D.Enqueue(Entry);
}

void SRenderer::Draw2DDisintegrate(UVec3 Position, const SSpriteHandle& SpriteHandle, const SSpriteHandle& NoiseHandle,
    float Progress)
{
    SEntry2D Entry;
    Entry.Program2DType = EProgram2DType::Uber2D;
    Entry.Position = Position;
    Entry.SizePixels = SpriteHandle.Sprite->SizePixels;
    Entry.UVRect = SpriteHandle.Sprite->UVRect;

    Entry.Mode = SEntryMode{
        UBER2D_MODE_DISINTEGRATE,
        { Progress, 0.0f, 0.0f, 0.0f },
        { NoiseHandle.Sprite->UVRect }
    };

    Queue2D.Enqueue(Entry);
}

void SRenderer::Draw3D(UVec3 Position, SGeometry* Geometry)
{
    SEntry3D Entry;

    Entry.Geometry = Geometry;
    Entry.Model = UMat4x4::Identity();
    Entry.Model.Translate(Position);

    Entry.Mode = SEntryMode{
        UBER3D_MODE_BASIC
    };

    Queue3D.Enqueue(Entry);
}

void SRenderer::Draw3DLevel(SLevel& Level, const UVec2Int& POVOrigin, const SDirection& POVDirection)
{
    auto constexpr DrawDistanceForward = 4;
    auto constexpr DrawDistanceSide = 2;

    auto& DoorDrawCall = LevelDrawData.DrawCalls[ETileGeometryType::Door];

    /* @TODO: Generic CleanDynamic method? */
    DoorDrawCall.DynamicCount = 0;

    if (Level.DirtyFlags & ELevelDirtyFlags::DrawSet)
    {
        LevelDrawData.Clear();

        auto& FloorDrawCall = LevelDrawData.DrawCalls[ETileGeometryType::Floor];

        auto& HoleDrawCall = LevelDrawData.DrawCalls[ETileGeometryType::Hole];

        auto& WallDrawCall = LevelDrawData.DrawCalls[ETileGeometryType::Wall];

        auto& WallJointDrawCall = LevelDrawData.DrawCalls[ETileGeometryType::WallJoint];

        auto& DoorFrameDrawCall = LevelDrawData.DrawCalls[ETileGeometryType::DoorFrame];

        if (!Level.IsValidTile(POVOrigin))
        {
            return;
        }

        auto POVDirectionInverted = POVDirection.Inverted();
        auto POVDirectionVectorForward = POVDirection.GetVector<int>();
        auto POVDirectionVectorSide = UVec2Int{ POVDirectionVectorForward.Y, -POVDirectionVectorForward.X };

        for (int SideCounter = -DrawDistanceSide; SideCounter <= DrawDistanceSide; ++SideCounter)
        {
            for (int ForwardCounter = -1; ForwardCounter < DrawDistanceForward; ++ForwardCounter)
            {
                auto RelativeX = (POVDirectionVectorForward.X * ForwardCounter) + (POVDirectionVectorSide.X * SideCounter);
                auto RelativeY = (POVDirectionVectorForward.Y * ForwardCounter) + (POVDirectionVectorSide.Y * SideCounter);

                auto X = POVOrigin.X + RelativeX;
                auto Y = POVOrigin.Y + RelativeY;

                auto XOffset = (float)X;
                auto YOffset = (float)Y;

                auto TileCoords = UVec2Int{ X, Y };

                auto TileTransform = UMat4x4::Identity();
                TileTransform.Translate(UVec3{ XOffset, 0.0f, YOffset });

                /* @TODO: Draw joints in separate loop. */
                /* @TODO: Maybe don't store them at all? */
                if (Level.bUseWallJoints && Level.IsValidWallJoint({ X, Y }))
                {
                    auto bWallJoint = Level.IsWallJointAt({ X, Y });
                    if (bWallJoint)
                    {
                        WallJointDrawCall.Push(TileTransform);
                    }
                }

                auto Tile = Level.GetTileAt(TileCoords);

                if (Tile == nullptr)
                {
                    continue;
                }

                if (SideCounter < 0 && ForwardCounter == 0 && !Tile->IsEdgeEmpty(POVDirection.Side().Inverted()))
                {
                    continue;
                }

                if (SideCounter > 0 && ForwardCounter == 0 && !Tile->IsEdgeEmpty(POVDirection.Side()))
                {
                    continue;
                }

                if (SideCounter == 0 && ForwardCounter >= 1 && !Tile->IsEdgeEmpty(POVDirection.Inverted()))
                {
                    break;
                }

                if (Tile->CheckFlag(TILE_FLOOR_BIT))
                {
                    FloorDrawCall.Push(TileTransform);
                }
                else if (Tile->CheckFlag(TILE_HOLE_BIT))
                {
                    HoleDrawCall.Push(TileTransform);
                }

                for (auto& Direction : SDirection::All())
                {
                    if (Tile->IsEdgeEmpty(Direction))
                    {
                        continue;
                    }

                    auto Transform = TileTransform;
                    Transform.Rotate(Direction.RotationFromDirection(),
                        { 0.0f, 1.0f, 0.0f });

                    if (Tile->CheckEdgeFlag(TILE_EDGE_WALL_BIT, Direction))
                    {
                        WallDrawCall.Push(Transform);
                    }

                    if (Tile->CheckEdgeFlag(TILE_EDGE_DOOR_BIT, Direction))
                    {
                        DoorFrameDrawCall.Push(Transform);

                        /* Check two adjacent tiles for ongoing door animation.
                         * Prevents static doors from being drawn if the animation is playing. */
                        if (Level.DoorInfo.Timeline.IsPlaying())
                        {
                            if (TileCoords == Level.DoorInfo.TileCoords && Direction == Level.DoorInfo.Direction)
                            {
                                continue;
                            }
                            if (TileCoords == POVOrigin && Direction == POVDirectionInverted)
                            {
                                continue;
                            }
                        }

                        Draw3DLevelDoor(DoorDrawCall, TileCoords, Direction, -1.0f);
                    }
                }
            }
        }
        Level.DirtyFlags &= ~ELevelDirtyFlags::DrawSet;

        Log::Draw<ELogLevel::Debug>("%s(): Regenerated Level Draw Set", __func__);
    }

    Draw3DLevelDoor(
        DoorDrawCall,
        Level.DoorInfo.TileCoords,
        Level.DoorInfo.Direction,
        Level.DoorInfo.Timeline.Value);

    SEntry3D Entry;

    Entry.Geometry = LevelDrawData.TileSet;
    Entry.Model = UMat4x4::Identity();
    Entry.InstancedDrawCall = &LevelDrawData.DrawCalls[0];
    Entry.InstancedDrawCallCount = ETileGeometryType::Count;

    Entry.Mode = SEntryMode{
        UBER3D_MODE_LEVEL
    };

    Queue3D.Enqueue(Entry);
}

void SRenderer::Draw3DLevelDoor(SInstancedDrawCall& DoorDrawCall, const UVec2Int& TileCoords, SDirection Direction, float AnimationAlpha) const
{
    if (TileCoords.X + TileCoords.Y < 0)
    {
        return;
    }

    auto Temp = SDirection{ Direction }.GetVector<float>();
    UVec3 DirectionalOffset{};
    DirectionalOffset.X = Temp.X;
    DirectionalOffset.Z = Temp.Y;

    auto TileCoordsOffset = UVec3{ (float)TileCoords.X, 0.0f, (float)TileCoords.Y };

    UMat4x4 Transform = UMat4x4::Identity();
    Transform.Translate((DirectionalOffset * 0.5f) + TileCoordsOffset);

    switch (LevelDrawData.TileSet->DoorAnimationType)
    {
        case EDoorAnimationType::TwoDoors:
        {
            /* Right door. */
            auto RightTransform = Transform;

            RightTransform.Rotate(SDirection{ Direction }.Inverted().RotationFromDirection(), { 0.0f, 1.0f, 0.0f });
            RightTransform.Translate({ LevelDrawData.TileSet->DoorOffset, 0.0f, 0.0f });

            /* Left door. */
            auto LeftTransform = Transform;

            LeftTransform.Rotate(Math::PI, { 0.0f, 1.0f, 0.0f });
            LeftTransform.Rotate(SDirection{ Direction }.Inverted().RotationFromDirection(), { 0.0f, 1.0f, 0.0f });
            LeftTransform.Translate({ LevelDrawData.TileSet->DoorOffset, 0.0f, 0.0f });

            /* Animate relevant doors. */
            if (AnimationAlpha > 0.0f)
            {
                auto A = AnimationAlpha * Math::HalfPI;

                LeftTransform.Rotate(A);
                //                A = std::max(0.0f, A - 0.4f);
                //                A *= 1.35f;
                RightTransform.Rotate(-A);

                DoorDrawCall.PushDynamic(LeftTransform);
                DoorDrawCall.PushDynamic(RightTransform);
            }
            else if (AnimationAlpha < 0.0f)
            {
                DoorDrawCall.Push(LeftTransform);
                DoorDrawCall.Push(RightTransform);
            }
        }
        break;

        default:
            break;
    }
}

void STexture::InitFromPixels(int Width, int Height, bool bAlpha, const void* Pixels)
{
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

void STexture::InitEmpty(int Width, int Height, bool bAlpha)
{
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

void STexture::InitFromRawImage(const CRawImage& RawImage)
{
    InitFromPixels(RawImage.Width, RawImage.Height, RawImage.Channels == 4, RawImage.Data);
}

void STexture::Cleanup()
{
    glDeleteTextures(1, &ID);

    Log::Draw<ELogLevel::Debug>("Deleting STexture", "");
}

void STexture::BindToTextureUnit(int TextureUnit) const
{
    glActiveTexture(GL_TEXTURE0 + TextureUnit);
    glBindTexture(GL_TEXTURE_2D, ID);
}

void SAtlas::Init(int InTextureUnitID)
{
    TextureUnitID = InTextureUnitID;
    InitEmpty(WidthAndHeight, WidthAndHeight, true);
    std::iota(SortingIndices.begin(), SortingIndices.end(), 0);
}

SSpriteHandle SAtlas::AddSprite(const SAsset& Resource)
{
    CRawImageInfo const RawImageInfo(Resource);

    Sprites[CurrentIndex].SizePixels = { RawImageInfo.Width, RawImageInfo.Height };
    Sprites[CurrentIndex].Resource = &Resource;
    return { this, &Sprites[CurrentIndex++] };
}

void SAtlas::Build()
{
    BindToTextureUnit(TextureUnitID);

    std::sort(SortingIndices.begin(), SortingIndices.end(), [&](const int& IndexA, const int& IndexB) {
        return Sprites[IndexA].SizePixels.Y > Sprites[IndexB].SizePixels.Y;
    });

    int CursorX{};
    int CursorY{};
    int MaxHeight{};

    for (int Index = 0; Index < CurrentIndex; ++Index)
    {
        auto& Sprite = Sprites[SortingIndices[Index]];
        CRawImage const Image(*Sprite.Resource);

        if (CursorX + Image.Width > WidthAndHeight)
        {
            CursorY += MaxHeight;
            CursorX = 0;
            MaxHeight = 0;
        }

        if (CursorY + Image.Height > WidthAndHeight)
        {
            break;
        }

        float MinU = (float)(CursorX) / (float)(WidthAndHeight);
        float MinV = (float)(CursorY) / (float)(WidthAndHeight);
        float MaxU = MinU + ((float)(Image.Width) / (float)WidthAndHeight);
        float MaxV = MinV + ((float)(Image.Height) / (float)WidthAndHeight);
        Sprite.UVRect = { MinU, MinV, MaxU, MaxV };

        glTexSubImage2D(GL_TEXTURE_2D, 0,
            CursorX, CursorY,
            Image.Width, Image.Height,
            GL_RGBA,
            GL_UNSIGNED_BYTE, Image.Data);

        CursorX += Image.Width;

        if (Image.Height > MaxHeight)
        {
            MaxHeight = Image.Height;
        }
    }
}
