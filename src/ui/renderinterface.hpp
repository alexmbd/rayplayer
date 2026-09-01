#pragma once

#include "RmlUi/Core/RenderInterface.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include <unordered_map>

namespace Rayplayer
{
class RenderInterface : public Rml::RenderInterface
{
  public:
    Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) override;
    void RenderGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle texture) override;
    void ReleaseGeometry(Rml::CompiledGeometryHandle geometry) override;

    Rml::TextureHandle LoadTexture(Rml::Vector2i &texture_dimensions, const Rml::String &source) override;
    Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions) override;
    void ReleaseTexture(Rml::TextureHandle texture) override;

    void EnableScissorRegion(bool enable) override;
    void SetScissorRegion(Rml::Rectanglei region) override;

    void SetTransform(const Rml::Matrix4f *transform) override;

  private:
    struct CompiledGeometry
    {
        Rml::Vector<Rml::Vertex> vertices;
        Rml::Vector<int> indices;
    };

    std::unordered_map<Rml::CompiledGeometryHandle, CompiledGeometry> m_geometries;
    std::unordered_map<Rml::TextureHandle, Texture2D> m_textures;

    Rml::CompiledGeometryHandle m_nextGeometryHandle{1};
    Rml::TextureHandle m_nextTextureHandle{1};

    bool m_transformActive{false};
    Matrix m_currentTransform{MatrixIdentity()};
};
}