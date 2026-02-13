#include "sfg_trajectory_planner/grid.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>

#include "sfg_trajectory_planner/core/gfx/renderer.hpp"
#include "sfg_trajectory_planner/core/serialization/abstract_serializer.hpp"

namespace sfg_trajectory_planner
{
    Grid::Grid(core::SceneObject::ConstructionKey key, core::Scene &scene, uuids::uuid uuid) : SceneObject(key, scene, uuid) {}

    void Grid::serialize(core::serialization::AbstractSerializer *serializer) const
    {
        SceneObject::serialize(serializer);

        serializer->serialize("grid_size", std::vector<float>{m_grid_size.x, m_grid_size.y});
        serializer->serialize("grid_spacing", std::vector<float>{m_grid_spacing});
        serializer->serialize("color", std::vector<float>{m_color.r, m_color.g, m_color.b});
    }

    void Grid::deserialize(core::serialization::AbstractSerializer *serializer)
    {
        SceneObject::deserialize(serializer);

        auto grid_size = std::get<std::vector<float>>(serializer->deserialize("grid_size"));
        m_grid_size = glm::vec3(grid_size[0], grid_size[1], grid_size[2]);
        m_grid_spacing = std::get<float>(serializer->deserialize("grid_spacing"));
        auto color = std::get<std::vector<float>>(serializer->deserialize("color"));
        m_color = glm::vec3(color[0], color[1], color[2]);
    }

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