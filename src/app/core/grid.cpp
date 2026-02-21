#include "sfg_trajectory_planner/app/core/grid.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>

#include "sfg_trajectory_planner/engine/core/gfx/renderer.hpp"
#include "sfg_trajectory_planner/engine/core/gfx/utils.hpp"
#include "sfg_trajectory_planner/engine/core/serialization/abstract_serializer.hpp"

namespace sfg_trajectory_planner::app::core
{
    Grid::Grid(engine::core::SceneObject::ConstructionKey key, engine::core::Scene &scene, uuids::uuid uuid) : SceneObject(key, scene, uuid)
    {
    }

    void Grid::serialize(engine::core::serialization::AbstractSerializer *serializer) const
    {
        SceneObject::serialize(serializer);

        serializer->serialize("grid_size", std::vector<float>{m_grid_size.x, m_grid_size.y});
        serializer->serialize("grid_spacing", m_grid_spacing);
        serializer->serialize("color", std::vector<float>{m_color.r, m_color.g, m_color.b});
    }

    void Grid::deserialize(engine::core::serialization::AbstractSerializer *serializer)
    {
        SceneObject::deserialize(serializer);

        auto grid_size = serializer->deserialize<std::vector<float>>("grid_size");
        m_grid_size = glm::vec2(grid_size[0], grid_size[1]);
        m_grid_spacing = serializer->deserialize<float>("grid_spacing");
        auto color = serializer->deserialize<std::vector<float>>("color");
        m_color = glm::vec3(color[0], color[1], color[2]);
    }

    void Grid::render_object(engine::core::gfx::Renderer &renderer)
    {
        using namespace engine::core::gfx::utils;

        auto ls_to_ws_matrix = get_ls_to_ws_matrix();
        auto extents = 0.5f * m_grid_size;

        // Draw outer rectangle.
        renderer.add_line(ls_to_ws_matrix, -extents.x * s_right - extents.y * s_forward, extents.x * s_right - extents.y * s_forward, m_color);
        renderer.add_line(ls_to_ws_matrix, extents.x * s_right - extents.y * s_forward, extents.x * s_right + extents.y * s_forward, m_color);
        renderer.add_line(ls_to_ws_matrix, extents.x * s_right + extents.y * s_forward, -extents.x * s_right + extents.y * s_forward, m_color);
        renderer.add_line(ls_to_ws_matrix, -extents.x * s_right + extents.y * s_forward, -extents.x * s_right - extents.y * s_forward, m_color);

        // Draw horizontal lines.
        for (auto z = 0.0f; z < extents.y; z += m_grid_spacing)
        {
            renderer.add_line(ls_to_ws_matrix, -extents.x * s_right - z * s_forward, extents.x * s_right - z * s_forward, 0.5f * m_color);
            renderer.add_line(ls_to_ws_matrix, -extents.x * s_right + z * s_forward, extents.x * s_right + z * s_forward, 0.5f * m_color);
        }

        // Draw vertical lines.
        for (auto x = 0.0f; x < extents.x; x += m_grid_spacing)
        {
            renderer.add_line(ls_to_ws_matrix, -x * s_right - extents.y * s_forward, -x * s_right + extents.y * s_forward, 0.5f * m_color);
            renderer.add_line(ls_to_ws_matrix, x * s_right - extents.y * s_forward, x * s_right + extents.y * s_forward, 0.5f * m_color);
        }
    }

    glm::vec2 Grid::get_grid_size() const
    {
        return m_grid_size;
    }

    float Grid::get_grid_spacing() const
    {
        return m_grid_spacing;
    }

    glm::vec3 Grid::get_color() const
    {
        return m_color;
    }

    void Grid::set_grid_size(const glm::vec2 &grid_size)
    {
        m_grid_size = glm::max(grid_size, 0.1f);
    }

    void Grid::set_grid_spacing(float grid_spacing)
    {
        m_grid_spacing = glm::max(grid_spacing, 0.1f);
    }

    void Grid::set_color(const glm::vec3 &color)
    {
        m_color = color;
    }
}