#include "sfg_trajectory_planner/engine/core/gfx/text_font.hpp"

#include <fstream>
#include <iostream>
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

namespace sfg_trajectory_planner::engine::core::gfx
{
    const std::uint32_t TextFont::s_bitmap_width = 512;
    const std::uint32_t TextFont::s_bitmap_height = 512;

    TextFont::TextFont(const std::string &filepath, float height) : m_baked_height(height)
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

        if (!file.read((char *)data.data(), size))
        {
            return;
        }

        *this = TextFont(data.data(), height);
    }

    TextFont::TextFont(std::uint8_t *data, float height) : m_baked_height(height)
    {
        std::vector<unsigned char> temp_bitmap(s_bitmap_width * s_bitmap_height);

        stbtt_BakeFontBitmap(
            data,
            0,
            height,
            temp_bitmap.data(),
            s_bitmap_width,
            s_bitmap_height,
            32,
            96,
            m_character_data);

        glGenTextures(1, &m_font_atlas);
        glBindTexture(GL_TEXTURE_2D, m_font_atlas);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, s_bitmap_width, s_bitmap_height, 0, GL_RED, GL_UNSIGNED_BYTE, temp_bitmap.data());
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

    void TextFont::get_character_quad(unsigned char character, float *x, float *y, stbtt_aligned_quad *quad)
    {
        if (character >= 32 && character < 128)
        {
            stbtt_GetBakedQuad(m_character_data, s_bitmap_width, s_bitmap_height, character - 32, x, y, quad, 1);
        }
    }

    float TextFont::get_baked_height() const
    {
        return m_baked_height;
    }
}