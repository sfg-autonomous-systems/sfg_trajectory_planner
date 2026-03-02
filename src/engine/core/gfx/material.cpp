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

        for (const auto &[location, texture] : m_textures)
        {
            glActiveTexture(GL_TEXTURE0 + texture.m_unit);
            glBindTexture(texture.m_type, texture.m_id);
            glUniform1i(location, static_cast<int>(texture.m_unit));
        }

        for (const auto &[location, value] : m_uniforms)
        {
            std::visit(
                [location](const auto &value)
                {
                    using ValueType = std::decay_t<decltype(value)>;

                    if constexpr (std::is_same_v<ValueType, int>)
                    {
                        glUniform1i(location, value);
                    }
                    else if constexpr (std::is_same_v<ValueType, float>)
                    {
                        glUniform1f(location, value);
                    }
                    else if constexpr (std::is_same_v<ValueType, glm::vec2>)
                    {
                        glUniform2fv(location, 1, glm::value_ptr(value));
                    }
                    else if constexpr (std::is_same_v<ValueType, glm::vec3>)
                    {
                        glUniform3fv(location, 1, glm::value_ptr(value));
                    }
                    else if constexpr (std::is_same_v<ValueType, glm::vec4>)
                    {
                        glUniform4fv(location, 1, glm::value_ptr(value));
                    }
                    else if constexpr (std::is_same_v<ValueType, glm::mat4>)
                    {
                        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
                    }
                },
                value);
        }
    }

    void Material::unbind() const
    {
        for (const auto &[name, texture] : m_textures)
        {
            glActiveTexture(GL_TEXTURE0 + texture.m_unit);
            glBindTexture(texture.m_type, 0);
        }
        m_shader->unbind();
    }

    void Material::set_uniform(const std::string &name, const UniformValue &value)
    {
        auto location = m_shader->get_uniform_location(name);

        if (location != -1)
        {
            m_uniforms[location] = value;
        }
    }

    void Material::set_texture(const std::string &name, std::uint32_t texture_id, std::uint32_t texture_type, std::uint32_t texture_unit)
    {
        auto location = m_shader->get_uniform_location(name);

        if (location != -1)
        {
            m_textures[location] = TextureValue{texture_id, texture_type, texture_unit};
        }
    }

    Shader *Material::get_shader() const
    {
        return m_shader.get();
    }
}