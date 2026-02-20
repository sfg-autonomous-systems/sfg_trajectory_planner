#pragma once

#include <glad/glad.h>

namespace sfg_trajectory_planner::engine::core::gfx
{
    class Shader
    {
    public:
        Shader(const char *vertex_source, const char *fragment_source);
        ~Shader();
        Shader(const Shader &) = delete;
        Shader &operator=(const Shader &) = delete;
        Shader(Shader &&) noexcept;
        Shader &operator=(Shader &&) noexcept;

        void bind() const;
        void unbind() const;
        GLuint get_id() const;

    private:
        GLuint m_id = 0;
    };
}