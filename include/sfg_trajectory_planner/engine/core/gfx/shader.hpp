#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

#include "sfg_trajectory_planner/engine/core/gfx/bindable.hpp"

namespace sfg_trajectory_planner::engine::core::gfx
{
    class Shader : public IBindable
    {
    public:
        Shader(const std::string &shader_source);
        Shader(const Shader &) = delete;
        Shader &operator=(const Shader &) = delete;
        Shader(Shader &&) noexcept;
        Shader &operator=(Shader &&) noexcept;
        ~Shader();

        void bind() const override;
        void unbind() const override;
        GLuint get_id() const;
        GLint get_uniform_location(const std::string &name) const;

    private:
        struct Stage
        {
            const char *m_name;
            const char *m_keyword;
            GLuint m_shader_type;
        };

        static constexpr Stage s_stages[] = {
            {"Vertex", "#pragma stage vertex", GL_VERTEX_SHADER},
            {"Geometry", "#pragma stage geometry", GL_GEOMETRY_SHADER},
            {"Fragment", "#pragma stage fragment", GL_FRAGMENT_SHADER}};
        static constexpr size_t s_stage_count = sizeof(s_stages) / sizeof(Stage);

        GLuint m_id = 0;
        mutable std::unordered_map<std::string, GLint> m_uniform_location_cache;
    };
}