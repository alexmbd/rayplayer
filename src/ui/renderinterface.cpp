#include "renderinterface.hpp"

namespace Rayplayer
{
Rml::CompiledGeometryHandle RenderInterface::CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices)
{
    CompiledGeometry geom;
    geom.vertices.assign(vertices.begin(), vertices.end());
    geom.indices.assign(indices.begin(), indices.end());

    Rml::CompiledGeometryHandle handle = m_nextGeometryHandle++;
    m_geometries.emplace(handle, std::move(geom));
    return handle;
}

void RenderInterface::RenderGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle texture)
{
    if (!m_geometries.contains(geometry)) { return; }

    const CompiledGeometry &geom = m_geometries[geometry];

    Texture2D *tex               = nullptr;
    if (texture != 0 && m_textures.contains(texture)) { tex = &m_textures[texture]; }

    if (tex) { rlSetTexture(tex->id); }
    else
    {
        rlSetTexture(rlGetTextureIdDefault());
    }

    if (m_transformActive)
    {
        rlPushMatrix();
        rlMultMatrixf(MatrixToFloat(m_currentTransform));
    }

    rlBegin(RL_TRIANGLES);
    for (int idx : geom.indices)
    {
        const Rml::Vertex &v = geom.vertices[idx];

        rlColor4ub(v.colour.red, v.colour.green, v.colour.blue, v.colour.alpha);
        rlTexCoord2f(v.tex_coord.x, v.tex_coord.y);
        rlVertex2f(v.position.x + translation.x, v.position.y + translation.y);
    }
    rlEnd();

    if (m_transformActive) { rlPopMatrix(); }

    rlSetTexture(0);
}

void RenderInterface::ReleaseGeometry(Rml::CompiledGeometryHandle geometry) { m_geometries.erase(geometry); }

Rml::TextureHandle RenderInterface::LoadTexture(Rml::Vector2i &texture_dimensions, const Rml::String &source)
{
    Image img = LoadImage(source.c_str());
    if (img.data == nullptr) { return 0; }

    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);

    texture_dimensions.x      = tex.width;
    texture_dimensions.y      = tex.height;

    Rml::TextureHandle handle = m_nextTextureHandle++;
    m_textures.emplace(handle, tex);
    return handle;
}

Rml::TextureHandle RenderInterface::GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions)
{
    Image img{};
    img.data                  = (void *)source.data();
    img.width                 = source_dimensions.x;
    img.height                = source_dimensions.y;
    img.mipmaps               = 1;
    img.format                = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

    Texture2D tex             = LoadTextureFromImage(img);

    Rml::TextureHandle handle = m_nextTextureHandle++;
    m_textures.emplace(handle, tex);
    return handle;
}

void RenderInterface::ReleaseTexture(Rml::TextureHandle texture)
{
    if (!m_textures.contains(texture)) { return; }
    UnloadTexture(m_textures[texture]);
    m_textures.erase(texture);
}

void RenderInterface::EnableScissorRegion(bool enable)
{
    rlDrawRenderBatchActive();
    if (enable) { rlEnableScissorTest(); }
    else
    {
        rlDisableScissorTest();
    }
}

void RenderInterface::SetScissorRegion(Rml::Rectanglei region)
{
    rlDrawRenderBatchActive();

    // rlgl scissor origin is bottom-left while RmlUi gives top-left
    int y = GetScreenHeight() - (region.Top() + region.Height());
    rlScissor(region.Left(), y, region.Width(), region.Height());
}

void RenderInterface::SetTransform(const Rml::Matrix4f *transform)
{
    rlDrawRenderBatchActive();
    if (transform)
    {
        m_transformActive = true;
        std::memcpy(&m_currentTransform, transform->data(), sizeof(float) * 16);
    }
    else
    {
        m_transformActive  = false;
        m_currentTransform = MatrixIdentity();
    }
}
}