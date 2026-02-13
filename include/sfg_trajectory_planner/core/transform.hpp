#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace sfg_trajectory_planner::core::serialization
{
    class AbstractSerializer;
}

namespace sfg_trajectory_planner::core
{
    class Transform
    {
    public:
        Transform();
        Transform(const glm::mat4 &matrix);
        Transform(const glm::vec3 &translation, const glm::quat &rotation, const glm::vec3 &scale);

        void serialize(serialization::AbstractSerializer *serializer) const;
        void deserialize(serialization::AbstractSerializer *serializer);

        glm::mat4 get_matrix() const;
        glm::vec3 get_translation() const;
        glm::vec3 get_euler_angles() const;
        glm::quat get_rotation() const;
        glm::vec3 get_scale() const;

        void set_matrix(const glm::mat4 &matrix);
        void set_translation(glm::vec3 translation);
        void set_rotation(glm::quat rotation);
        void set_euler_angles(glm::vec3 euler_angles);
        void set_scale(glm::vec3 scale);

        void render_inspector(bool render_translation = true, bool render_rotation = true, bool render_scale = true, bool render_labels = true);

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