#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace sfg_trajectory_planner::engine::core::gfx
{
    class FrameBuffer
    {
    public:
        FrameBuffer(const glm::vec3 &clear_color, glm::ivec2 size);
        FrameBuffer(const FrameBuffer &) = delete;
        FrameBuffer &operator=(const FrameBuffer &) = delete;
        FrameBuffer(FrameBuffer &&) noexcept;
        FrameBuffer &operator=(FrameBuffer &&) noexcept;
        ~FrameBuffer();

        GLuint get_color_texture() const;

        void bind();
        void unbind();
        void clear();
        void resize_if_needed(glm::ivec2 size);

    private:
        glm::vec3 m_clear_color;
        glm::ivec2 m_size = {0, 0};

        GLuint m_fbo = 0;
        GLuint m_color_texture = 0;
        GLuint m_depth_rbo = 0;
    };
}