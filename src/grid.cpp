#include "sfg_trajectory_planner/grid.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>

namespace sfg_trajectory_planner
{
    Grid::Grid(core::SceneObjectKey key, core::Scene &scene, uuids::uuid uuid) : SceneObject(key, scene, uuid) {}

    void Grid::render_object(core::gfx::Renderer &renderer)
    {
        const glm::vec3 color = {1.0f, 1.0f, 1.0f};

        auto model_matrix = get_global_transform();
        // Ignore scale.
        model_matrix[0][0] = model_matrix[1][1] = model_matrix[2][2] = 1.0f;

        auto extents = 0.5f * glm::vec3(m_grid_size.x, 0.0f, m_grid_size.y);

        // Draw outer rectangle.
        renderer.add_line(model_matrix, {-extents.x, 0.0f, -extents.z}, {extents.x, 0.0f, -extents.z}, color);
        renderer.add_line(model_matrix, {extents.x, 0.0f, -extents.z}, {extents.x, 0.0f, extents.z}, color);
        renderer.add_line(model_matrix, {extents.x, 0.0f, extents.z}, {-extents.x, 0.0f, extents.z}, color);
        renderer.add_line(model_matrix, {-extents.x, 0.0f, extents.z}, {-extents.x, 0.0f, -extents.z}, color);

        // Draw horizontal lines.
        for (auto z = 0.0f; z < extents.z; z += m_grid_spacing)
        {
            renderer.add_line(model_matrix, {-extents.x, 0.0f, -z}, {extents.x, 0.0f, -z}, 0.5f * color);
            renderer.add_line(model_matrix, {-extents.x, 0.0f, z}, {extents.x, 0.0f, z}, 0.5f * color);
        }

        // Draw vertical lines.
        for (auto x = 0.0f; x < extents.x; x += m_grid_spacing)
        {
            renderer.add_line(model_matrix, {-x, 0.0f, -extents.z}, {-x, 0.0f, extents.z}, 0.5f * color);
            renderer.add_line(model_matrix, {x, 0.0f, -extents.z}, {x, 0.0f, extents.z}, 0.5f * color);
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
    }
}