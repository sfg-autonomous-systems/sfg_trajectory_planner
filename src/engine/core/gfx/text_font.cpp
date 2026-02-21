#include "sfg_trajectory_planner/engine/core/gfx/text_font.hpp"

#include <fstream>
#include <iostream>
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

namespace sfg_trajectory_planner::engine::core::gfx
{
    TextFont::TextFont(const std::filesystem::path &filepath, float height) : m_baked_height(height)
    {
        std::ifstream file(filepath, std::ios::binary | std::ios::ate);

        if (!file.is_open())
        {
            std::cerr << "Failed to open font file: " << filepath << std::endl;
            return;
        }
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        std::vector<unsigned char> data(size);

        if (!file.read(reinterpret_cast<char *>(data.data()), size))
        {
            return;
        }

        *this = TextFont(data.data(), height);
    }

    TextFont::TextFont(std::uint8_t *data, float height) : m_baked_height(height)
    {
        std::vector<unsigned char> font_atlas(s_font_atlas_width * s_font_atlas_height);

        stbtt_BakeFontBitmap(
            data,
            0,
            height,
            font_atlas.data(),
            s_font_atlas_width,
            s_font_atlas_height,
            s_first_character,
            s_character_count,
            m_character_data);

        glGenTextures(1, &m_font_atlas);
        glBindTexture(GL_TEXTURE_2D, m_font_atlas);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, s_font_atlas_width, s_font_atlas_height, 0, GL_RED, GL_UNSIGNED_BYTE, font_atlas.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    TextFont::TextFont(TextFont &&other) noexcept : m_font_atlas(other.m_font_atlas), m_baked_height(other.m_baked_height)
    {
        std::copy(std::begin(other.m_character_data), std::end(other.m_character_data), std::begin(m_character_data));
        other.m_font_atlas = 0;
    }

    TextFont &TextFont::operator=(TextFont &&other) noexcept
    {
        if (this != &other)
        {
            if (m_font_atlas)
            {
                glDeleteTextures(1, &m_font_atlas);
            }
            m_font_atlas = other.m_font_atlas;
            std::copy(std::begin(other.m_character_data), std::end(other.m_character_data), std::begin(m_character_data));
            m_baked_height = other.m_baked_height;
            other.m_font_atlas = 0;
        }
        return *this;
    }

    TextFont::~TextFont()
    {
        if (m_font_atlas)
        {
            glDeleteTextures(1, &m_font_atlas);
        }
    }

    GLuint TextFont::get_font_atlas() const
    {
        return m_font_atlas;
    }

    void TextFont::get_character_quad(unsigned char character, float *x, float *y, stbtt_aligned_quad *quad) const
    {
        if (character >= s_first_character && character < s_first_character + s_character_count)
        {
            stbtt_GetBakedQuad(m_character_data, s_font_atlas_width, s_font_atlas_height, character - s_first_character, x, y, quad, 1);
        }
    }

    float TextFont::get_baked_height() const
    {
        return m_baked_height;
    }

    glm::vec2 TextFont::calculate_text_size(std::string_view text) const
    {
        if (text.empty())
        {
            return {0.0f, 0.0f};
        }

        stbtt_aligned_quad quad;
        auto x = 0.0f;
        auto y = 0.0f;
        auto min_x = std::numeric_limits<float>::max(), max_x = std::numeric_limits<float>::lowest();
        auto min_y = std::numeric_limits<float>::max(), max_y = std::numeric_limits<float>::lowest();

        for (char character : text)
        {
            get_character_quad(character, &x, &y, &quad);
            min_x = std::min(min_x, quad.x0);
            max_x = std::max(max_x, quad.x1);
            min_y = std::min(min_y, quad.y0);
            max_y = std::max(max_y, quad.y1);
        }
        return {max_x - min_x, max_y - min_y};
    }

    glm::vec2 TextFont::text_anchor_to_offset(std::string_view text, TextAnchor anchor) const
    {
        auto text_size = calculate_text_size(text);

        switch (anchor)
        {
            case TextAnchor::TopLeft:
                return {+0.0f * text_size.x, +1.0f * text_size.y};
            case TextAnchor::TopCenter:
                return {-0.5f * text_size.x, +1.0f * text_size.y};
            case TextAnchor::TopRight:
                return {-1.0f * text_size.x, +1.0f * text_size.y};
            case TextAnchor::CenterLeft:
                return {+0.0f * text_size.x, +0.5f * text_size.y};
            case TextAnchor::Center:
                return {-0.5f * text_size.x, +0.5f * text_size.y};
            case TextAnchor::CenterRight:
                return {-1.0f * text_size.x, +0.5f * text_size.y};
            case TextAnchor::BottomLeft:
                return {+0.0f * text_size.x, -0.0f * text_size.y};
            case TextAnchor::BottomCenter:
                return {-0.5f * text_size.x, -0.0f * text_size.y};
            case TextAnchor::BottomRight:
                return {-1.0f * text_size.x, -0.0f * text_size.y};
            default:
                return {0.0f, 0.0f};
        }
    }
}