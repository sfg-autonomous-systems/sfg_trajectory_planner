#pragma once

#include <rclcpp/rclcpp.hpp>

#include "sfg_agent_msgs/msg/trajectory.hpp"
#include "sfg_trajectory_planner/engine/editor/scene_object_editor.hpp"

namespace sfg_trajectory_planner::app::core
{
    class Trajectory;
}

namespace sfg_trajectory_planner::app::editor
{
    class TrajectoryEditor : public engine::editor::SceneObjectEditor<core::Trajectory>
    {
    public:
        TrajectoryEditor(const engine::editor::EditorContext &editor_context, rclcpp::Node *node);
        bool render_editor(engine::core::gfx::Renderer &renderer) override;
        bool render_inspector() override;
        void publish_trajectory(rclcpp::Time time);

    private:
        void create_trajectory_publisher(const std::string &topic_name);

        size_t m_selected_waypoint_index = std::numeric_limits<size_t>::max();
        rclcpp::Publisher<sfg_agent_msgs::msg::Trajectory>::SharedPtr m_trajectory_publisher;
        rclcpp::Node *m_node;
    };
}