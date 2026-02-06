#include "sfg_trajectory_planner/transform_inspector.hpp"

#include <glm/gtc/type_ptr.hpp>

#include "sfg_trajectory_planner/gfx_math.hpp"

namespace sfg_trajectory_planner
{
    TransformInspector::TransformInspector() : m_translation(0.0f), m_rotation(0.0f), m_scale(1.0f) {}

    void TransformInspector::render_internal()
    {
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("Position:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::DragFloat3("##position", glm::value_ptr(m_translation));

            ImGui::Text("Rotation:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::DragFloat3("##rotation", glm::value_ptr(m_rotation));

            ImGui::Text("Scale:   ");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::DragFloat3("##scale", glm::value_ptr(m_scale));
            ImGui::Spacing();
        }
    }

    void TransformInspector::set_translation(const glm::vec3 &translation)
    {
        m_translation = translation;
    }

    void TransformInspector::set_rotation(const glm::vec3 &rotation)
    {
        m_rotation = rotation;
    }

    void TransformInspector::set_scale(const glm::vec3 &scale)
    {
        m_scale = scale;
    }

    glm::vec3 TransformInspector::get_translation() const
    {
        return m_translation;
    }

    glm::vec3 TransformInspector::get_rotation() const
    {
        return m_rotation;
    }

    glm::vec3 TransformInspector::get_scale() const
    {
        return m_scale;
    }

    glm::mat4 TransformInspector::get_transform_matrix() const
    {
        glm::mat4 translation_matrix = glm::translate(glm::mat4(1.0f), m_translation);
        glm::mat4 rotation_matrix = glm::mat4(1.0f);
        rotation_matrix = glm::rotate(rotation_matrix, glm::radians(m_rotation.y), gfx_math::up.xyz());
        rotation_matrix = glm::rotate(rotation_matrix, glm::radians(m_rotation.x), gfx_math::right.xyz());
        rotation_matrix = glm::rotate(rotation_matrix, glm::radians(m_rotation.z), gfx_math::forward.xyz());
        glm::mat4 scale_matrix = glm::scale(glm::mat4(1.0f), m_scale);
        return translation_matrix * rotation_matrix * scale_matrix;
    }
}