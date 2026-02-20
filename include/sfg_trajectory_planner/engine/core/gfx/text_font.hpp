#pragma once

#include <vector>
#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <stb_truetype.h>

namespace sfg_trajectory_planner::engine::core::gfx
{
    class TextFont
    {
    public:
        TextFont(const std::string &filepath, float height = 64.0f);
        TextFont(std::uint8_t *data, float height = 64.0f);
        TextFont(const TextFont &) = delete;
        TextFont &operator=(const TextFont &) = delete;
        TextFont(TextFont &&) noexcept;
        TextFont &operator=(TextFont &&) noexcept;
        ~TextFont();

        GLuint get_font_atlas() const;
        void get_character_quad(unsigned char character, float *x, float *y, stbtt_aligned_quad *quad);
        float get_baked_height() const;

    private:
        static const std::uint32_t s_bitmap_width;
        static const std::uint32_t s_bitmap_height;

        GLuint m_font_atlas = 0;
        stbtt_bakedchar m_character_data[96];
        float m_baked_height;
    };
}