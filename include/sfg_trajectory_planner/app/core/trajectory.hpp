#pragma once

#include "sfg_trajectory_planner/app/core/waypoint.hpp"
#include "sfg_trajectory_planner/engine/core/scene_object.hpp"

namespace sfg_trajectory_planner::app::core
{
    class Trajectory : public engine::core::SceneObject
    {
    public:
        Trajectory(engine::core::SceneObject::ConstructionKey key, engine::core::Scene &scene, uuids::uuid uuid);
        void serialize(engine::core::serialization::AbstractSerializer *serializer) const override;
        void deserialize(engine::core::serialization::AbstractSerializer *serializer) override;
        void render_object(engine::core::gfx::Renderer &renderer) override;

        const std::string &get_topic_name() const;
        const std::string &get_frame_id() const;
        float get_time_from_start() const;
        glm::vec3 get_color() const;
        size_t get_waypoint_count() const;

        void set_topic_name(std::string topic_name);
        void set_frame_id(std::string frame_id);
        void set_time_from_start(float time_from_start);
        void set_color(glm::vec3 color);

        bool empty() const;

        // Waypoint manipulation methods.
        void add_waypoint();
        template <typename... Args>
        void add_waypoint(Args &&...args);
        void remove_waypoint(size_t index);

        const engine::core::Transform &get_waypoint_transform_ls(size_t index) const;
        float get_waypoint_time_from_last(size_t index) const;
        Waypoint::Constraints get_waypoint_constraints(size_t index) const;

        void set_waypoint_transform_ls(size_t index, engine::core::Transform transform_ls);
        void set_waypoint_time_from_last(size_t index, float time_from_last);
        void set_waypoint_constraints(size_t index, Waypoint::Constraints constraints);

        bool can_translate_waypoint(size_t index) const;
        bool can_rotate_waypoint(size_t index) const;
        bool can_scale_waypoint(size_t index) const;

    private:
        void enforce_waypoint_constraints(size_t index);

        std::string m_topic_name = "/trajectory";
        std::string m_frame_id = "base_link";
        float m_time_from_start = 0.0f;
        glm::vec3 m_color = {0.0f, 1.0f, 0.0f};
        std::vector<Waypoint> m_waypoints;
    };
}

#include "sfg_trajectory_planner/app/core/trajectory.tpp"