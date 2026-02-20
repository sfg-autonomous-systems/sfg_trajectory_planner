#include "sfg_trajectory_planner/engine/core/gfx/framebuffer.hpp"

#include <stdexcept>

namespace sfg_trajectory_planner::engine::core::gfx
{
    FrameBuffer::FrameBuffer(const glm::vec3 &clear_color, glm::ivec2 size)
        : m_clear_color(clear_color)
    {
        resize_if_needed(size);
    }

    FrameBuffer::~FrameBuffer()
    {
        if (m_fbo)
        {
            glDeleteFramebuffers(1, &m_fbo);
        }

        if (m_color_texture)
        {
            glDeleteTextures(1, &m_color_texture);
        }

        if (m_depth_rbo)
        {
            glDeleteRenderbuffers(1, &m_depth_rbo);
        }
    }

    FrameBuffer::FrameBuffer(FrameBuffer &&other) noexcept
        : m_clear_color(other.m_clear_color),
          m_fbo(other.m_fbo),
          m_color_texture(other.m_color_texture),
          m_depth_rbo(other.m_depth_rbo)
    {
        other.m_fbo = 0;
        other.m_color_texture = 0;
        other.m_depth_rbo = 0;
    }

    FrameBuffer &FrameBuffer::operator=(FrameBuffer &&other) noexcept
    {
        if (this != &other)
        {
            m_clear_color = other.m_clear_color;
            m_fbo = other.m_fbo;
            m_color_texture = other.m_color_texture;
            m_depth_rbo = other.m_depth_rbo;

            other.m_fbo = 0;
            other.m_color_texture = 0;
            other.m_depth_rbo = 0;
        }
        return *this;
    }

    GLuint FrameBuffer::get_color_texture() const
    {
        return m_color_texture;
    }

    void FrameBuffer::bind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glViewport(0, 0, m_size.x, m_size.y);
    }

    void FrameBuffer::unbind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void FrameBuffer::clear()
    {
        glClearColor(m_clear_color.r, m_clear_color.g, m_clear_color.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void FrameBuffer::resize_if_needed(glm::ivec2 size)
    {
        if (size == m_size)
        {
            return;
        }
        m_size = std::move(size);

        if (m_fbo)
        {
            glDeleteFramebuffers(1, &m_fbo);
            glDeleteTextures(1, &m_color_texture);
            glDeleteRenderbuffers(1, &m_depth_rbo);
        }

        glGenFramebuffers(1, &m_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

        glGenTextures(1, &m_color_texture);
        glBindTexture(GL_TEXTURE_2D, m_color_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_size.x, m_size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_color_texture, 0);

        glGenRenderbuffers(1, &m_depth_rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, m_depth_rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_size.x, m_size.y);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depth_rbo);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}