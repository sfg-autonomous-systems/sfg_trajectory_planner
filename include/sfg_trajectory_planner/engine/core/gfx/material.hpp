#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>

#include "sfg_trajectory_planner/engine/core/gfx/bindable.hpp"

namespace sfg_trajectory_planner::engine::core::gfx
{
    class Shader;
}

namespace sfg_trajectory_planner::engine::core::gfx
{
    class Material : public IBindable
    {
    public:
        using UniformValue = std::variant<int, float, glm::vec2, glm::vec3, glm::vec4, glm::mat4>;

        struct TextureValue
        {
            // The ID of the texture object as returned by glGenTextures.
            std::uint32_t m_id;
            // The texture target, one of...
            // - GL_TEXTURE_1D
            // - GL_TEXTURE_2D
            // - GL_TEXTURE_3D
            // - GL_TEXTURE_1D_ARRAY
            // - GL_TEXTURE_2D_ARRAY
            // - GL_TEXTURE_RECTANGLE
            // - GL_TEXTURE_CUBE_MAP
            // - GL_TEXTURE_CUBE_MAP_ARRAY
            // - GL_TEXTURE_BUFFER
            // - GL_TEXTURE_2D_MULTISAMPLE
            // - GL_TEXTURE_2D_MULTISAMPLE_ARRAY
            std::uint32_t m_type;
            // The texture unit to which to bind the texture, where 0 corresponds to GL_TEXTURE0.
            std::uint32_t m_unit;
        };

        Material(std::shared_ptr<Shader> shader);
        void bind() const override;
        void unbind() const override;
        void set_uniform(const std::string &name, const UniformValue &value);
        void set_texture(const std::string &name, std::uint32_t id, std::uint32_t target, std::uint32_t unit = 0);
        Shader *get_shader() const;

    private:
        std::shared_ptr<Shader> m_shader;
        std::unordered_map<std::string, UniformValue> m_uniforms;
        std::unordered_map<std::string, TextureValue> m_textures;
    };
}