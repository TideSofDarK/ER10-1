#pragma once

#include <vector>
#include <array>

#include "glm/glm.hpp"
#include "glm/mat4x4.hpp"
#include "CommonTypes.hpp"
#include "Resource.h"

#define RENDERER_QUEUE2D_SIZE 16
#define RENDERER_QUEUE3D_SIZE 16

struct SLevel;

struct SProgram {
private:
    static unsigned int CreateVertexShader(TResource Data, int Length);

    static unsigned int CreateFragmentShader(TResource Data, int Length);

    static unsigned int CreateProgram(unsigned int VertexShader, unsigned int FragmentShader);

protected:
    virtual void InitUniforms() = 0;

public:
    unsigned int ID{};

    void Init(TResource VertexShaderData, int VertexShaderLength, TResource FragmentShaderData, int FragmentShaderLength);

    void Cleanup() const;

    void Use() const;
};

struct SProgram2D : SProgram {
protected:
    void InitUniforms() override;

public:
    int UniformBlockCommon2D{};
    int UniformModeID{};
    int UniformModeControlAID{};
    int UniformPositionScreenSpaceID{};
    int UniformSizeScreenSpaceID{};
    int UniformColorTextureID{};
};

struct SProgram3D : SProgram {
protected:
    void InitUniforms() override;

public:
    int UniformBlockCommon3D{};
    int UniformModelID{};
};

struct SFrameBuffer {
    unsigned FBO{};
    unsigned ColorID{};
    unsigned DepthID{};
    glm::vec3 ClearColor;

    void Init(int Width, int Height, glm::vec3 InClearColor);

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

    virtual void Cleanup();
};

struct SLevelGeometry : SGeometry {
private:
    static std::vector<glm::vec3> Vertices;
    static std::vector<unsigned short> Indices;
public:
    void InitFromLevel(const SLevel &Level);
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
    unsigned ID;

    void InitFromPixels(int Width, int Height, bool bAlpha, const void *Pixels);

    void Cleanup();
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

enum class EHUDMode {
    BorderDashed,
    Button
};

enum class ESimple2DMode {
    Texture,
    Haze,
    BackBlur
};

enum class EProgram2DType {
    HUD,
    Simple2D,
};

struct SEntry2D {
    STexture *Texture{};
    glm::vec3 Position{};
    glm::vec2 Size{};
    EProgram2DType Program2DType{};
    int Mode;
    glm::vec4 ModeControlA;
};

struct SEntry3D {
    SGeometry *Geometry{};
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

struct SRenderer {
    SRenderQueue<SEntry2D, RENDERER_QUEUE2D_SIZE> Queue2D;
    SRenderQueue<SEntry3D, RENDERER_QUEUE3D_SIZE> Queue3D;
    SProgram2D SimpleProgram2D;
    SProgram2D HUDProgram2D;
    SProgram3D SimpleProgram3D;
    SFrameBuffer MainFrameBuffer;
    SGeometry Quad2D;

    void Init(int Width, int Height);

    void Cleanup();

    void UploadProjectionAndViewFromCamera(const SCamera &Camera) const;

    void Flush(const SWindowData &WindowData);

#pragma region Queue_2D_API

    void DrawHUD(glm::vec3 Position, glm::vec2 Size, EHUDMode Mode);

    void Draw2D(glm::vec3 Position, glm::vec2 Size, STexture *Texture);

    void Draw2DEx(glm::vec3 Position, glm::vec2 Size, STexture *Texture, ESimple2DMode Mode);

    void Draw2DEx(glm::vec3 Position, glm::vec2 Size, STexture *Texture, ESimple2DMode Mode, glm::vec4 ModeControlA);

    void Draw2DBackBlur(glm::vec3 Position, glm::vec2 Size, STexture *Texture, float Count, float Speed, float Step);

#pragma endregion

#pragma region Queue_3D_API

    void Draw3D(glm::vec3 Position, SGeometry *Geometry);

#pragma endregion
};


