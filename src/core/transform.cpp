#include "sfg_trajectory_planner/core/transform.hpp"

#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>

#include "sfg_trajectory_planner/core/serialization/abstract_serializer.hpp"

namespace sfg_trajectory_planner::core
{
    Transform::Transform()
        : m_matrix(1.0f),
          m_translation(0.0f),
          m_rotation(0.0f, 0.0f, 0.0f, 1.0f),
          m_euler_angles(0.0f),
          m_scale(1.0f),
          m_dirty(false) {}

    Transform::Transform(const glm::mat4 &matrix) : m_matrix(matrix), m_dirty(false)
    {
        update_components();
    }

    Transform::Transform(const glm::vec3 &translation, const glm::quat &rotation, const glm::vec3 &scale)
        : m_translation(translation),
          m_rotation(rotation),
          m_scale(scale),
          m_dirty(false)
    {
        update_matrix();
    }

    void Transform::serialize(serialization::AbstractSerializer *serializer) const
    {
        serializer->serialize("translation", std::vector<float>{m_translation.x, m_translation.y, m_translation.z});
        serializer->serialize("rotation", std::vector<float>{m_rotation.x, m_rotation.y, m_rotation.z, m_rotation.w});
        serializer->serialize("scale", std::vector<float>{m_scale.x, m_scale.y, m_scale.z});
    }

    void Transform::deserialize(serialization::AbstractSerializer *serializer)
    {
        auto translation = std::get<std::vector<float>>(serializer->deserialize("translation"));
        m_translation = glm::vec3(translation[0], translation[1], translation[2]);
        auto rotation = std::get<std::vector<float>>(serializer->deserialize("rotation"));
        m_rotation = glm::quat(rotation[3], rotation[0], rotation[1], rotation[2]);
        auto scale = std::get<std::vector<float>>(serializer->deserialize("scale"));
        m_scale = glm::vec3(scale[0], scale[1], scale[2]);

        update_matrix();
    }

    glm::mat4 Transform::get_matrix() const
    {
        if (m_dirty)
        {
            update_matrix();
        }
        return m_matrix;
    }

    glm::vec3 Transform::get_translation() const
    {
        return m_translation;
    }

    glm::quat Transform::get_rotation() const
    {
        return m_rotation;
    }

    glm::vec3 Transform::get_euler_angles() const
    {
        return m_euler_angles;
    }

    glm::vec3 Transform::get_scale() const
    {
        return m_scale;
    }

    void Transform::set_matrix(const glm::mat4 &matrix)
    {
        m_matrix = matrix;
        m_dirty = false;
        update_components();
    }

    void Transform::set_translation(glm::vec3 translation)
    {
        m_translation = translation;
        m_dirty = true;
    }

    void Transform::set_rotation(glm::quat rotation)
    {
        m_rotation = rotation;
        m_euler_angles = glm::degrees(glm::eulerAngles(rotation));
        m_dirty = true;
    }

    void Transform::set_euler_angles(glm::vec3 euler_angles)
    {
        m_euler_angles = euler_angles;
        m_rotation = glm::quat(glm::radians(euler_angles));
        m_dirty = true;
    }

    void Transform::set_scale(glm::vec3 scale)
    {
        const auto min_scale = 0.001f;

        m_scale = glm::max(scale, glm::vec3(min_scale));
        m_dirty = true;
    }

    void Transform::render_inspector(bool render_translation, bool render_rotation, bool render_scale, bool render_labels)
    {
        if (!render_labels)
        {
            ImGui::PushItemWidth(-1.0f);
        }

        if (render_translation && ImGui::DragFloat3(render_labels ? "Position [m]" : "##position", glm::value_ptr(m_translation), 0.1f))
        {
            set_translation(m_translation);
        }
        ImGui::SetItemTooltip("Position [m]");

        if (render_rotation && ImGui::DragFloat3(render_labels ? "Rotation [deg]" : "##rotation", glm::value_ptr(m_euler_angles), 0.1f))
        {
            set_euler_angles(m_euler_angles);
        }
        ImGui::SetItemTooltip("Rotation [deg]");

        if (render_scale && ImGui::DragFloat3(render_labels ? "Scale" : "##scale", glm::value_ptr(m_scale), 0.1f))
        {
            set_scale(m_scale);
        }
        ImGui::SetItemTooltip("Scale");

        if (!render_labels)
        {
            ImGui::PopItemWidth();
        }
    }

    void Transform::update_matrix() const
    {
        m_matrix = glm::scale(glm::translate(glm::mat4(1.0f), m_translation) * glm::toMat4(m_rotation), m_scale);
        m_dirty = false;
    }

    void Transform::update_components()
    {
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(m_matrix, m_scale, m_rotation, m_translation, skew, perspective);
        m_euler_angles = glm::degrees(glm::eulerAngles(m_rotation));
    }

}