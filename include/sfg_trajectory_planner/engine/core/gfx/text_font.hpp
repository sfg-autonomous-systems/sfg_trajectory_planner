#pragma once

#include <filesystem>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <stb_truetype.h>
#include <vector>

#include "sfg_trajectory_planner/engine/core/gfx/text_anchor.hpp"

namespace sfg_trajectory_planner::engine::core::gfx
{
    class TextFont
    {
    public:
        TextFont(const std::filesystem::path &filepath, float height = 64.0f);
        TextFont(std::uint8_t *data, float height = 64.0f);
        TextFont(const TextFont &) = delete;
        TextFont &operator=(const TextFont &) = delete;
        TextFont(TextFont &&) noexcept;
        TextFont &operator=(TextFont &&) noexcept;
        ~TextFont();

        GLuint get_font_atlas() const;
        void get_character_quad(unsigned char character, float *x, float *y, stbtt_aligned_quad *quad) const;
        float get_baked_height() const;

        glm::vec2 calculate_text_size(std::string_view text) const;
        glm::vec2 text_anchor_to_offset(std::string_view text, TextAnchor anchor) const;

    private:
        static constexpr size_t s_first_character = 32;
        static constexpr size_t s_character_count = 96;
        static constexpr std::uint32_t s_font_atlas_width = 512;
        static constexpr std::uint32_t s_font_atlas_height = 512;

        GLuint m_font_atlas = 0;
        stbtt_bakedchar m_character_data[s_character_count];
        float m_baked_height;
    };
}