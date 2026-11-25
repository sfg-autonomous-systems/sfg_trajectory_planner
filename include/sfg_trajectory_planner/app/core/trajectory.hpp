#pragma once

#include <memory>
#include <mutex>
#include <nav2_msgs/action/follow_path.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>

#include "sfg_trajectory_planner/app/core/waypoint.hpp"
#include "sfg_trajectory_planner/engine/core/scene_object.hpp"

namespace sfg_trajectory_planner::engine::core::assets
{
    class AssetLocator;
}

namespace sfg_trajectory_planner::app::core
{
    class Trajectory : public engine::core::SceneObject, public std::enable_shared_from_this<Trajectory>
    {
    public:
        Trajectory(engine::core::SceneObject::ConstructionKey key, const engine::core::Scene &scene, uuids::uuid uuid, const engine::core::assets::AssetLocator &asset_locator, rclcpp::Node *node);
        void serialize(engine::core::serialization::AbstractSerializer *serializer) const override;
        void deserialize(engine::core::serialization::AbstractSerializer *serializer) override;
        void render_object(engine::core::gfx::Renderer &renderer) override;

        const std::string &get_action_name() const;
        const std::string &get_frame_id() const;
        glm::vec3 get_color() const;
        size_t get_waypoint_count() const;

        void set_action_name(std::string action_name);
        void set_frame_id(std::string frame_id);
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

        void follow_trajectory();
        void cancel_follow_trajectory();
        bool follow_trajectory_active() const;
        std::string get_follow_trajectory_status() const;

    private:
        void create_follow_trajectory_client();
        void enforce_waypoint_constraints(size_t index);

        std::string m_action_name = "/follow_path";
        std::string m_frame_id = "base_link";
        glm::vec3 m_color = {0.0f, 1.0f, 0.0f};
        std::vector<Waypoint> m_waypoints;
        float m_path_spacing = 0.05f;

        rclcpp_action::Client<nav2_msgs::action::FollowPath>::SharedPtr m_follow_trajectory_client;
        rclcpp_action::Client<nav2_msgs::action::FollowPath>::GoalHandle::SharedPtr m_follow_trajectory_goal_handle;
        mutable std::mutex m_follow_trajectory_mutex;
        std::string m_follow_trajectory_status;
        rclcpp::Node *m_node;
    };
}

#include "sfg_trajectory_planner/app/core/trajectory.tpp"