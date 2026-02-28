#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>

namespace sfg_trajectory_planner::engine::core::gfx
{
    class Shader;
}

namespace sfg_trajectory_planner::engine::core::gfx
{
    class Material
    {
    public:
        using UniformValue = std::variant<int, float, glm::vec2, glm::vec3, glm::vec4, glm::mat4>;

        Material(std::shared_ptr<Shader> shader);

        void set_uniform(const std::string &name, const UniformValue &value);

    private:
        std::shared_ptr<Shader> m_shader;
        std::unordered_map<std::string, UniformValue> m_uniforms;
    };
}