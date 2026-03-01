#pragma once

#include <glad/glad.h>

#include "sfg_trajectory_planner/engine/core/gfx/bindable.hpp"

namespace sfg_trajectory_planner::engine::core::gfx
{
    class Shader : public IBindable
    {
    public:
        Shader(const char *vertex_source, const char *fragment_source);
        Shader(const Shader &) = delete;
        Shader &operator=(const Shader &) = delete;
        Shader(Shader &&) noexcept;
        Shader &operator=(Shader &&) noexcept;
        ~Shader();

        void bind() const override;
        void unbind() const override;
        GLuint get_id() const;

    private:
        GLuint m_id = 0;
    };
}