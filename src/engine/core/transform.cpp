#include "sfg_trajectory_planner/engine/core/transform.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <imgui/imgui.h>

#include "sfg_trajectory_planner/engine/core/serialization/abstract_serializer.hpp"

namespace sfg_trajectory_planner::engine::core
{
    Transform::Transform()
        : m_matrix(1.0f),
          m_translation(0.0f),
          m_rotation(0.0f, 0.0f, 0.0f, 1.0f),
          m_euler_angles(0.0f),
          m_scale(1.0f),
          m_dirty(false)
    {
    }

    Transform::Transform(glm::mat4 matrix) : m_matrix(matrix), m_dirty(false)
    {
        update_components();
    }

    Transform::Transform(glm::vec3 translation, glm::quat rotation, glm::vec3 scale)
        : m_translation(std::move(translation)),
          m_rotation(std::move(rotation)),
          m_scale(std::move(scale)),
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
        auto translation = serializer->deserialize<std::vector<float>>("translation");
        m_translation = glm::vec3(translation[0], translation[1], translation[2]);
        auto rotation = serializer->deserialize<std::vector<float>>("rotation");
        m_rotation = glm::quat(rotation[3], rotation[0], rotation[1], rotation[2]);
        auto scale = serializer->deserialize<std::vector<float>>("scale");
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

    void Transform::set_matrix(glm::mat4 matrix)
    {
        m_matrix = std::move(matrix);
        m_dirty = false;
        update_components();
    }

    void Transform::set_translation(glm::vec3 translation)
    {
        m_translation = std::move(translation);
        m_dirty = true;
    }

    void Transform::set_euler_angles(glm::vec3 euler_angles)
    {
        m_euler_angles = std::move(euler_angles);
        m_rotation = glm::quat(glm::radians(m_euler_angles));
        m_dirty = true;
    }

    void Transform::set_rotation(glm::quat rotation)
    {
        m_rotation = std::move(rotation);
        m_euler_angles = glm::degrees(glm::eulerAngles(m_rotation));
        m_dirty = true;
    }

    void Transform::set_scale(glm::vec3 scale)
    {
        const auto min_scale = 0.001f;

        m_scale = glm::max(scale, glm::vec3(min_scale));
        m_dirty = true;
    }

    Transform &Transform::translate(glm::vec3 translation, bool local)
    {
        if (local)
        {
            set_translation(m_translation + m_rotation * translation);
        }
        else
        {
            set_translation(m_translation + translation);
        }
        return *this;
    }

    Transform &Transform::rotate(glm::vec3 euler_angles, bool local)
    {
        return rotate(glm::quat(glm::radians(euler_angles)), local);
    }

    Transform &Transform::rotate(glm::quat rotation, bool local)
    {
        if (local)
        {
            set_rotation(rotation * m_rotation);
        }
        else
        {
            set_rotation(m_rotation * rotation);
        }
        return *this;
    }

    Transform &Transform::scale(glm::vec3 scale)
    {
        set_scale(m_scale * scale);
        return *this;
    }

    Transform &Transform::look_in(glm::vec3 direction, glm::vec3 up)
    {
        const auto epsilon = 0.001f;

        if (glm::length2(direction) < epsilon)
        {
            return *this;
        }

        glm::vec3 forward = glm::normalize(direction);
        glm::vec3 left = glm::normalize(glm::cross(up, forward));
        glm::vec3 orthonormal_up = glm::cross(forward, left);
        glm::mat3 rotation_matrix = glm::mat3(forward, left, orthonormal_up);
        set_rotation(glm::quat_cast(rotation_matrix));
        return *this;
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