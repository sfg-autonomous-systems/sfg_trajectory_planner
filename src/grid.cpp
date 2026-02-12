#include "sfg_trajectory_planner/grid.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>

#include "sfg_trajectory_planner/core/gfx/renderer.hpp"

namespace sfg_trajectory_planner
{
    Grid::Grid(core::SceneObject::ConstructionKey key, core::Scene &scene, uuids::uuid uuid) : SceneObject(key, scene, uuid) {}

    void Grid::render_object(core::gfx::Renderer &renderer)
    {
        auto object_to_world_matrix = get_object_to_world_matrix();
        auto extents = 0.5f * glm::vec3(m_grid_size.x, 0.0f, m_grid_size.y);

        // Draw outer rectangle.
        renderer.add_line(object_to_world_matrix, {-extents.x, 0.0f, -extents.z}, {extents.x, 0.0f, -extents.z}, m_color);
        renderer.add_line(object_to_world_matrix, {extents.x, 0.0f, -extents.z}, {extents.x, 0.0f, extents.z}, m_color);
        renderer.add_line(object_to_world_matrix, {extents.x, 0.0f, extents.z}, {-extents.x, 0.0f, extents.z}, m_color);
        renderer.add_line(object_to_world_matrix, {-extents.x, 0.0f, extents.z}, {-extents.x, 0.0f, -extents.z}, m_color);

        // Draw horizontal lines.
        for (auto z = 0.0f; z < extents.z; z += m_grid_spacing)
        {
            renderer.add_line(object_to_world_matrix, {-extents.x, 0.0f, -z}, {extents.x, 0.0f, -z}, 0.5f * m_color);
            renderer.add_line(object_to_world_matrix, {-extents.x, 0.0f, z}, {extents.x, 0.0f, z}, 0.5f * m_color);
        }

        // Draw vertical lines.
        for (auto x = 0.0f; x < extents.x; x += m_grid_spacing)
        {
            renderer.add_line(object_to_world_matrix, {-x, 0.0f, -extents.z}, {-x, 0.0f, extents.z}, 0.5f * m_color);
            renderer.add_line(object_to_world_matrix, {x, 0.0f, -extents.z}, {x, 0.0f, extents.z}, 0.5f * m_color);
        }
    }

    void Grid::render_inspector_internal()
    {
        if (ImGui::DragFloat2("Grid Size [m]", glm::value_ptr(m_grid_size), 0.1f))
        {
            m_grid_size = glm::max(m_grid_size, 0.0f);
        }

        if (ImGui::DragFloat("Grid Spacing [m]", &m_grid_spacing, 0.1f))
        {
            m_grid_spacing = glm::max(m_grid_spacing, 0.1f);
        }

        ImGui::ColorEdit3("Grid Color", glm::value_ptr(m_color));
    }
}