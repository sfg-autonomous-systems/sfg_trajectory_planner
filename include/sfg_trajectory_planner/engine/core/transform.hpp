#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "sfg_trajectory_planner/engine/core/gfx/utils.hpp"
#include "sfg_trajectory_planner/engine/core/serialization/serializable.hpp"

namespace sfg_trajectory_planner::engine::core
{
    class Transform : public serialization::ISerializable
    {
    public:
        Transform();
        Transform(glm::mat4 matrix);
        Transform(glm::vec3 translation, glm::quat rotation, glm::vec3 scale);

        void serialize(serialization::AbstractSerializer *serializer) const override;
        void deserialize(serialization::AbstractSerializer *serializer) override;

        glm::mat4 get_matrix() const;
        glm::vec3 get_translation() const;
        glm::vec3 get_euler_angles() const;
        glm::quat get_rotation() const;
        glm::vec3 get_scale() const;

        void set_matrix(glm::mat4 matrix);
        void set_translation(glm::vec3 translation);
        void set_euler_angles(glm::vec3 euler_angles);
        void set_rotation(glm::quat rotation);
        void set_scale(glm::vec3 scale);

        Transform &translate(glm::vec3 translation, bool local = true);
        Transform &rotate(glm::vec3 euler_angles, bool local = true);
        Transform &rotate(glm::quat rotation, bool local = true);
        Transform &scale(glm::vec3 scale);

        Transform &look_in(glm::vec3 direction, glm::vec3 up = gfx::utils::s_up.xyz());

    private:
        void update_matrix() const;
        void update_components();

        mutable glm::mat4 m_matrix;
        glm::vec3 m_translation;
        glm::quat m_rotation;
        glm::vec3 m_euler_angles;
        glm::vec3 m_scale;
        mutable bool m_dirty;
    };
}