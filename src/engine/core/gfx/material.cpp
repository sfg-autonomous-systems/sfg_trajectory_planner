#include "sfg_trajectory_planner/engine/core/gfx/material.hpp"

#include <yaml-cpp/yaml.h>

#include "sfg_trajectory_planner/engine/core/gfx/shader.hpp"

namespace sfg_trajectory_planner::engine::core::gfx
{
    Material::Material(std::shared_ptr<Shader> shader)
        : m_shader(std::move(shader))
    {
    }

    void Material::set_uniform(const std::string &name, const UniformValue &value)
    {
        m_uniforms[name] = value;
    }
}