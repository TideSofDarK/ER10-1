#include "Draw.hpp"

#include <iostream>
#include "glad/gl.h"
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include "Level.hpp"
#include "Constants.hpp"

EXTLD(HUD_vert)
EXTLD(HUD_frag)

EXTLD(Simple2D_vert)
EXTLD(Simple2D_frag)

EXTLD(Simple3D_vert)
EXTLD(Simple3D_frag)

unsigned int SProgram::CreateVertexShader(TResource Data, int Length) {
    unsigned int VertexShader = glCreateShader(GL_VERTEX_SHADER);
    const auto DataChar = reinterpret_cast<const char *>(Data);
    glShaderSource(VertexShader, 1, &DataChar, &Length);
    glCompileShader(VertexShader);
    int Success;
    glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &Success);
    if (!Success) {
        char Log[512];
        glGetShaderInfoLog(VertexShader, 512, nullptr, Log);
        std::cout << Log << std::endl;
    }
    return VertexShader;
}

unsigned int SProgram::CreateFragmentShader(TResource Data, int Length) {
    unsigned int FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const auto DataChar = reinterpret_cast<const char *>(Data);
    glShaderSource(FragmentShader, 1, &DataChar, &Length);
    glCompileShader(FragmentShader);
    int Success;
    glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &Success);
    if (!Success) {
        char Log[512];
        glGetShaderInfoLog(FragmentShader, 512, nullptr, Log);
        std::cout << Log << std::endl;
    }
    return FragmentShader;
}

unsigned int SProgram::CreateProgram(unsigned int VertexShader, unsigned int FragmentShader) {
    unsigned int Program = glCreateProgram();
    glAttachShader(Program, VertexShader);
    glAttachShader(Program, FragmentShader);
    glLinkProgram(Program);
    int Success;
    glGetProgramiv(Program, GL_LINK_STATUS, &Success);
    if (!Success) {
        char Log[512];
        glGetProgramInfoLog(Program, 512, nullptr, Log);
        printf("%s\n", Log);
    }
    return Program;
}

void SProgram::Init(TResource VertexShaderData, int VertexShaderLength, TResource FragmentShaderData,
                    int FragmentShaderLength) {
    unsigned int VertexShader = CreateVertexShader(VertexShaderData, VertexShaderLength);
    unsigned int FragmentShader = CreateFragmentShader(FragmentShaderData, FragmentShaderLength);
    ID = CreateProgram(VertexShader, FragmentShader);
    glDeleteShader(VertexShader);
    glDeleteShader(FragmentShader);
    InitUniforms();
}

void SProgram::Cleanup() const {
    glDeleteProgram(ID);

    std::cout << "Deleting SProgram..." << std::endl;
}

void SProgram::Use() const {
    glUseProgram(ID);
}

void SProgram3D::InitUniforms() {
    UniformBlockCommon3D = glGetUniformBlockIndex(ID, "ub_common");
    glUniformBlockBinding(ID, UniformBlockCommon3D, 0);
    UniformModelID = glGetUniformLocation(ID, "u_model");
}

void SProgram2D::InitUniforms() {
    UniformBlockCommon2D = glGetUniformBlockIndex(ID, "ub_common");
    glUniformBlockBinding(ID, UniformBlockCommon2D, 0);
    UniformModeID = glGetUniformLocation(ID, "u_mode");
    UniformModeControlAID = glGetUniformLocation(ID, "u_modeControlA");
    UniformPositionScreenSpaceID = glGetUniformLocation(ID, "u_positionScreenSpace");
    UniformSizeScreenSpaceID = glGetUniformLocation(ID, "u_sizeScreenSpace");
    UniformColorTextureID = glGetUniformLocation(ID, "u_colorTexture");
}

void SGeometry::Cleanup() {
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &CBO);
    glDeleteVertexArrays(1, &VAO);

    std::cout << "Deleting SGeometry with ElementCount == " + std::to_string(ElementCount) << std::endl;
}

std::vector<glm::vec3> SLevelGeometry::Vertices;
std::vector<unsigned short> SLevelGeometry::Indices;

void SLevelGeometry::InitFromLevel(const SLevel &Level) {
    ElementCount = 0;

    Vertices.reserve(Level.Width * Level.Height * 4);
    Vertices.clear();
    Indices.reserve(Level.Width * Level.Height * 6);
    Indices.clear();

    unsigned short CurrentQuadIndex = 0;
    for (unsigned int X = 0; X < Level.Width; ++X) {
        for (unsigned int Y = 0; Y < Level.Height; ++Y) {
            const auto Index = (Y * Level.Width) + X;
            const auto Tile = Level.Grid[Index];

            if (Tile) {
                auto XOffset = static_cast<float>(X);
                auto YOffset = static_cast<float>(Y);

                Indices.emplace_back(CurrentQuadIndex);
                Indices.emplace_back(CurrentQuadIndex + 1);
                Indices.emplace_back(CurrentQuadIndex + 2);
                Indices.emplace_back(CurrentQuadIndex);
                Indices.emplace_back(CurrentQuadIndex + 2);
                Indices.emplace_back(CurrentQuadIndex + 3);

                Vertices.emplace_back(0.5f + XOffset, 0.0f, 0.5f + YOffset);
                Vertices.emplace_back(0.5f + XOffset, 0.0f, -0.5f + YOffset);
                Vertices.emplace_back(-0.5f + XOffset, 0.0f, -0.5f + YOffset);
                Vertices.emplace_back(-0.5f + XOffset, 0.0f, 0.5f + YOffset);

                CurrentQuadIndex += 4;
            }
        }
    }

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, static_cast<long long>(Vertices.size() * sizeof(glm::vec3)), &Vertices[0],
                 GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    ElementCount = static_cast<int>(Indices.size());
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<long long>(ElementCount * sizeof(unsigned short)), &Indices[0],
                 GL_STATIC_DRAW);
}

void SCamera::Regenerate(float InFieldOfViewY, float InAspect) {
    FieldOfViewY = InFieldOfViewY;
    Aspect = InAspect;
    Projection = glm::perspective(glm::radians(FieldOfViewY), Aspect, 0.01f, 100.0f);
}

void SCamera::Update() {
    View = glm::lookAtRH(Position, Target, glm::vec3{0.0f, 1.0f, 0.0f});
}

void SFrameBuffer::Init(int Width, int Height, glm::vec3 InClearColor) {
    ClearColor = InClearColor;

    glGenTextures(1, &ColorID);
    glBindTexture(GL_TEXTURE_2D, ColorID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

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

    /** Load common programs */
    HUDProgram2D.Init(LDVAR(HUD_vert), LDLEN(HUD_vert), LDVAR(HUD_frag), LDLEN(HUD_frag));
    SimpleProgram2D.Init(LDVAR(Simple2D_vert), LDLEN(Simple2D_vert), LDVAR(Simple2D_frag), LDLEN(Simple2D_frag));
    SimpleProgram3D.Init(LDVAR(Simple3D_vert), LDLEN(Simple3D_vert), LDVAR(Simple3D_frag), LDLEN(Simple3D_frag));

    /** Initialize queues */
    Queue2D.Init(32);
    Queue2D.CommonUniformBlock.SetVector2(0, {static_cast<float>(Width), static_cast<float>(Height)});

    Queue3D.Init(sizeof(glm::mat4x4) * 2);

    /** Initialize framebuffers */
    MainFrameBuffer.Init(Width, Height, WINDOW_CLEAR_COLOR);
}

void SRenderer::Cleanup() {
    Queue2D.Cleanup();
    Queue3D.Cleanup();
    Quad2D.Cleanup();
    MainFrameBuffer.Cleanup();
    SimpleProgram2D.Cleanup();
    SimpleProgram3D.Cleanup();
}

void SRenderer::Flush(const SWindowData &WindowData) {
    /** Begin Draw */
    MainFrameBuffer.BindForDrawing();

    /** Draw 3D */
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glViewport(SCENE_OFFSET, SCENE_HEIGHT - SCENE_OFFSET, SCENE_WIDTH, SCENE_HEIGHT);

    Queue3D.CommonUniformBlock.Bind();

    glUseProgram(SimpleProgram3D.ID);

    auto ModelMatrix = glm::mat4(1.0);
    glUniformMatrix4fv(SimpleProgram3D.UniformModelID, 1, GL_FALSE, &ModelMatrix[0][0]);

    for (int Index = 0; Index < Queue3D.CurrentIndex; ++Index) {
        const auto &Entry = Queue3D.Entries[Index];
        if (Entry.Geometry == nullptr) {
            continue;
        }
        glBindVertexArray(Entry.Geometry->VAO);
        glDrawElements(GL_TRIANGLES, Entry.Geometry->ElementCount, GL_UNSIGNED_SHORT, nullptr);
    }

    /** Draw 2D */
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    Queue2D.CommonUniformBlock.Bind();

    Queue2D.CommonUniformBlock.SetFloat(8, WindowData.Seconds);

    glBindVertexArray(Quad2D.VAO);

    for (int Index = 0; Index < Queue2D.CurrentIndex; ++Index) {
        const auto &Entry = Queue2D.Entries[Index];

        const SProgram2D *Program = nullptr;
        switch (Entry.Program2DType) {
            case EProgram2DType::HUD:
                Program = &HUDProgram2D;
                break;
            case EProgram2DType::Simple2D:
                Program = &SimpleProgram2D;
            default:
                break;
        }
        glUseProgram(Program->ID);

        glUniform1i(Program->UniformModeID, Entry.Mode);
        glUniform2f(Program->UniformPositionScreenSpaceID, Entry.Position.x, Entry.Position.y);
        glUniform2f(Program->UniformSizeScreenSpaceID, Entry.Size.x, Entry.Size.y);

        if (Entry.Texture != nullptr) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, Entry.Texture->ID);
            glUniform1i(Program->UniformColorTextureID, 0);
        }

        switch (Entry.Program2DType) {
            case EProgram2DType::Simple2D:
                switch (static_cast<ESimple2DMode>(Entry.Mode)) {
                    case ESimple2DMode::BackBlur:
                        glUniform4fv(Program->UniformModeControlAID, 1, &Entry.ModeControlA[0]);
                        glDrawElementsInstanced(GL_TRIANGLES, Quad2D.ElementCount, GL_UNSIGNED_SHORT, nullptr, static_cast<int>(Entry.ModeControlA[0]));
                        glUniform1i(Program->UniformModeID, 0);
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }

        glDrawElements(GL_TRIANGLES, Quad2D.ElementCount, GL_UNSIGNED_SHORT, nullptr);
    }

    /** Display everything */
    MainFrameBuffer.BindForReading();
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glViewport(0, 0, WindowData.Width, WindowData.Height);
    glClearColor(0.0f, 0.125f, 0.125f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBlitFramebuffer(0, 0,
                      SCREEN_WIDTH, SCREEN_HEIGHT,
                      WindowData.BlitX, WindowData.BlitY,
                      WindowData.BlitWidth + WindowData.BlitX, WindowData.BlitHeight + WindowData.BlitY,
                      GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    Queue2D.Reset();
    Queue3D.Reset();
}

void SRenderer::UploadProjectionAndViewFromCamera(const SCamera &Camera) const {
    auto ProjectionMatrix = Camera.Projection;
    auto ViewMatrix = Camera.View;

    Queue3D.CommonUniformBlock.SetMatrix(0, ProjectionMatrix);
    Queue3D.CommonUniformBlock.SetMatrix(sizeof(glm::mat4x4), ViewMatrix);
}

void SRenderer::DrawHUD(glm::vec3 Position, glm::vec2 Size, EHUDMode Mode) {
    Queue2D.Enqueue({nullptr, glm::round(Position), glm::round(Size), EProgram2DType::HUD, static_cast<int>(Mode)});
}

void SRenderer::Draw2D(glm::vec3 Position, glm::vec2 Size, STexture *Texture) {
    Queue2D.Enqueue({Texture, glm::round(Position), glm::round(Size), EProgram2DType::Simple2D});
}

void SRenderer::Draw2DEx(glm::vec3 Position, glm::vec2 Size, STexture *Texture, ESimple2DMode Mode) {
    SEntry2D Entry;
    Entry.Position = Position;
    Entry.Size = Size;
    Entry.Texture = Texture;
    Entry.Mode = static_cast<int>(Mode);
    Entry.Program2DType = EProgram2DType::Simple2D;

    Queue2D.Enqueue(Entry);
}

void
SRenderer::Draw2DEx(glm::vec3 Position, glm::vec2 Size, STexture *Texture, ESimple2DMode Mode, glm::vec4 ModeControlA) {
    SEntry2D Entry;
    Entry.Position = Position;
    Entry.Size = Size;
    Entry.Texture = Texture;
    Entry.Mode = static_cast<int>(Mode);
    Entry.ModeControlA = ModeControlA;
    Entry.Program2DType = EProgram2DType::Simple2D;

    Queue2D.Enqueue(Entry);
}

void
SRenderer::Draw2DBackBlur(glm::vec3 Position, glm::vec2 Size, STexture *Texture, float Count, float Speed, float Step) {
    SEntry2D Entry;
    Entry.Position = Position;
    Entry.Size = Size;
    Entry.Texture = Texture;
    Entry.Mode = static_cast<int>(ESimple2DMode::BackBlur);
    Entry.ModeControlA.x = Count;
    Entry.ModeControlA.y = Speed;
    Entry.ModeControlA.z = Step;
    Entry.Program2DType = EProgram2DType::Simple2D;

    Queue2D.Enqueue(Entry);
}

void SRenderer::Draw3D(glm::vec3 Position, SGeometry *Geometry) {
    Queue3D.Enqueue({Geometry});
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
}

void STexture::Cleanup() {
    glDeleteTextures(1, &ID);

    std::cout << "Deleting STexture..." << std::endl;
}


