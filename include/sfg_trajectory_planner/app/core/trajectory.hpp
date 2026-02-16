#pragma once

#include "sfg_trajectory_planner/engine/core/scene_object.hpp"

namespace sfg_trajectory_planner::app::core
{
    class Trajectory : public engine::core::SceneObject
    {
    public:
        struct Waypoint
        {
            engine::core::Transform m_transform;
            float m_time_from_last = 1.0f;
        };

        Trajectory(engine::core::SceneObject::ConstructionKey key, engine::core::Scene &scene, uuids::uuid uuid);
        void serialize(engine::core::serialization::AbstractSerializer *serializer) const override;
        void deserialize(engine::core::serialization::AbstractSerializer *serializer) override;
        void render_object(engine::core::gfx::Renderer &renderer) override;

        const std::string &get_topic_name() const;
        const std::string &get_frame_id() const;
        float get_time_from_start() const;
        glm::vec3 get_color() const;
        std::vector<Waypoint> &get_waypoints();

        void set_topic_name(const std::string &topic_name);
        void set_frame_id(const std::string &frame_id);
        void set_time_from_start(float time_from_start);
        void set_color(const glm::vec3 &color);

    private:
        std::string m_topic_name = "/trajectory";
        std::string m_frame_id = "base_link";
        float m_time_from_start = 0.0f;
        glm::vec3 m_color = {0.0f, 1.0f, 0.0f};
        std::vector<Waypoint> m_waypoints;
    };
}