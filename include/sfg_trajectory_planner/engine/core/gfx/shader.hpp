#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

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
        void set_ls_to_ws_matrix(const glm::mat4 &matrix) const;
        void set_ws_to_cs_matrix(const glm::mat4 &matrix) const;
        void set_ls_to_cs_matrix(const glm::mat4 &matrix) const;
        GLuint get_id() const;
        GLint get_uniform_location(const std::string &name) const;

    private:
        GLuint m_id = 0;

        // Uniform locations cached for performance since these are set frequently.
        GLuint m_ls_to_ws_uniform_location = 0;
        GLuint m_ws_to_cs_uniform_location = 0;
        GLuint m_ls_to_cs_uniform_location = 0;

        mutable std::unordered_map<std::string, GLint> m_uniform_location_cache;
    };
}