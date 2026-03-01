#include "sfg_trajectory_planner/engine/core/gfx/material.hpp"

#include <glm/gtc/type_ptr.hpp>

#include "sfg_trajectory_planner/engine/core/gfx/shader.hpp"

namespace sfg_trajectory_planner::engine::core::gfx
{
    Material::Material(std::shared_ptr<Shader> shader) : m_shader(std::move(shader))
    {
    }

    void Material::bind() const
    {
        m_shader->bind();
        auto shader_id = m_shader->get_id();

        for (const auto &[name, value] : m_uniforms)
        {
            auto uniform_location = glGetUniformLocation(shader_id, name.c_str());

            if (uniform_location == -1)
            {
                continue;
            }

            std::visit(
                [&](const auto &value)
                {
                    using ValueType = std::decay_t<decltype(value)>;

                    if constexpr (std::is_same_v<ValueType, int>)
                    {
                        glUniform1i(uniform_location, value);
                    }
                    else if constexpr (std::is_same_v<ValueType, float>)
                    {
                        glUniform1f(uniform_location, value);
                    }
                    else if constexpr (std::is_same_v<ValueType, glm::vec2>)
                    {
                        glUniform2fv(uniform_location, 1, glm::value_ptr(value));
                    }
                    else if constexpr (std::is_same_v<ValueType, glm::vec3>)
                    {
                        glUniform3fv(uniform_location, 1, glm::value_ptr(value));
                    }
                    else if constexpr (std::is_same_v<ValueType, glm::vec4>)
                    {
                        glUniform4fv(uniform_location, 1, glm::value_ptr(value));
                    }
                    else if constexpr (std::is_same_v<ValueType, glm::mat4>)
                    {
                        glUniformMatrix4fv(uniform_location, 1, GL_FALSE, glm::value_ptr(value));
                    }
                },
                value);
        }
    }

    void Material::unbind() const
    {
        m_shader->unbind();
    }

    void Material::set_uniform(const std::string &name, const UniformValue &value)
    {
        m_uniforms[name] = value;
    }

    Shader *Material::get_shader() const
    {
        return m_shader.get();
    }
}