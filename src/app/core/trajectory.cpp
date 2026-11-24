#include "sfg_trajectory_planner/app/core/trajectory.hpp"

#include <algorithm>
#include <cmath>
#include <fmt/format.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <nav_msgs/msg/path.hpp>

#include "sfg_trajectory_planner/engine/core/gfx/camera.hpp"
#include "sfg_trajectory_planner/engine/core/gfx/renderer.hpp"
#include "sfg_trajectory_planner/engine/core/gfx/utils.hpp"
#include "sfg_trajectory_planner/engine/core/scene.hpp"
#include "sfg_trajectory_planner/engine/core/serialization/abstract_serializer.hpp"
#include "sfg_trajectory_planner/engine/editor/selection_context.hpp"

namespace sfg_trajectory_planner::app::core
{
    Trajectory::Trajectory(
        engine::core::SceneObject::ConstructionKey key,
        const engine::core::Scene &scene,
        uuids::uuid uuid,
        const engine::core::assets::AssetLocator &,
        rclcpp::Node *node)
        : SceneObject(key, scene, uuid),
          m_node(node)
    {
        create_follow_trajectory_client();
    }

    void Trajectory::serialize(engine::core::serialization::AbstractSerializer *serializer) const
    {
        SceneObject::serialize(serializer);

        serializer->serialize("action_name", m_action_name);
        serializer->serialize("frame_id", m_frame_id);
        serializer->serialize("color", std::vector<float>{m_color.r, m_color.g, m_color.b});
        serializer->begin_sequence("waypoints", engine::core::serialization::AbstractSerializer::Mode::Write);

        for (const auto &waypoint : m_waypoints)
        {
            serializer->next_item();
            waypoint.serialize(serializer);
        }
        serializer->end_sequence();
    }

    void Trajectory::deserialize(engine::core::serialization::AbstractSerializer *serializer)
    {
        SceneObject::deserialize(serializer);

        set_action_name(serializer->deserialize<std::string>("action_name"));
        m_frame_id = serializer->deserialize<std::string>("frame_id");
        auto color = serializer->deserialize<std::vector<float>>("color");
        m_color = glm::vec3(color[0], color[1], color[2]);

        auto waypoint_count = serializer->begin_sequence("waypoints", engine::core::serialization::AbstractSerializer::Mode::Read);
        m_waypoints.clear();
        m_waypoints.reserve(waypoint_count);

        for (size_t index = 0; index < waypoint_count; index++)
        {
            serializer->next_item();
            m_waypoints.emplace_back(Waypoint());
            m_waypoints.back().deserialize(serializer);
        }
        serializer->end_sequence();
    }

    void Trajectory::render_object(engine::core::gfx::Renderer &renderer)
    {
        auto ls_to_ws_matrix = get_ls_to_ws_matrix();

        for (size_t index = 1; index < m_waypoints.size(); index++)
        {
            renderer.add_line(
                glm::vec3(ls_to_ws_matrix * glm::vec4(m_waypoints[index - 1].m_transform_ls.get_translation(), 1.0f)),
                glm::vec3(ls_to_ws_matrix * glm::vec4(m_waypoints[index].m_transform_ls.get_translation(), 1.0f)),
                m_color);
        }

        auto time_from_start = 0.0f;

        for (size_t index = 0; index < m_waypoints.size(); index++)
        {
            time_from_start += m_waypoints[index].m_time_from_last;
            auto color = ImGui::GetStyleColorVec4(ImGuiCol_Text);

            renderer.add_text(
                ls_to_ws_matrix,
                m_waypoints[index].m_transform_ls.get_translation(),
                fmt::format("t+{:.1f}", time_from_start),
                2.0f * ImGui::GetFontSize(),
                glm::vec3(color.x, color.y, color.z),
                engine::core::gfx::TextAnchor::Center);
        }
    }

    const std::string &Trajectory::get_action_name() const
    {
        return m_action_name;
    }

    const std::string &Trajectory::get_frame_id() const
    {
        return m_frame_id;
    }

    glm::vec3 Trajectory::get_color() const
    {
        return m_color;
    }

    size_t Trajectory::get_waypoint_count() const
    {
        return m_waypoints.size();
    }

    void Trajectory::set_action_name(std::string action_name)
    {
        // ToDo: Validate action name.
        if (action_name == m_action_name)
        {
            return;
        }
        m_action_name = std::move(action_name);
        create_follow_trajectory_client();
    }

    void Trajectory::set_frame_id(std::string frame_id)
    {
        // ToDo: Validate frame ID.
        m_frame_id = std::move(frame_id);
    }

    void Trajectory::set_color(glm::vec3 color)
    {
        m_color = std::move(color);
    }

    bool Trajectory::empty() const
    {
        return m_waypoints.empty();
    }

    void Trajectory::add_waypoint()
    {
        // We'll place new waypoint's one unit forward from the last waypoint.
        auto last_transform_ls = m_waypoints.size() == 0 ? glm::mat4(1.0f) : m_waypoints.back().m_transform_ls;
        // We'll also inherit the constraints of the last waypoint.
        auto last_constraints = m_waypoints.size() == 0 ? core::Waypoint::Constraints::None : m_waypoints.back().m_constraints;

        add_waypoint(last_transform_ls.translate(engine::core::gfx::utils::s_forward.xyz()), 1.0f, last_constraints);
    }

    void Trajectory::remove_waypoint(size_t index)
    {
        if (index < m_waypoints.size())
        {
            m_waypoints.erase(m_waypoints.begin() + index);
        }
    }

    const engine::core::Transform &Trajectory::get_waypoint_transform_ls(size_t index) const
    {
        return m_waypoints[index].m_transform_ls;
    }

    float Trajectory::get_waypoint_time_from_last(size_t index) const
    {
        return m_waypoints[index].m_time_from_last;
    }

    Waypoint::Constraints Trajectory::get_waypoint_constraints(size_t index) const
    {
        return m_waypoints[index].m_constraints;
    }

    void Trajectory::set_waypoint_transform_ls(size_t index, engine::core::Transform transform_ls)
    {
        if (!can_translate_waypoint(index))
        {
            transform_ls.set_translation(m_waypoints[index].m_transform_ls.get_translation());
        }

        if (!can_rotate_waypoint(index))
        {
            transform_ls.set_rotation(m_waypoints[index].m_transform_ls.get_rotation());
        }

        if (!can_scale_waypoint(index))
        {
            transform_ls.set_scale(m_waypoints[index].m_transform_ls.get_scale());
        }
        m_waypoints[index].m_transform_ls = std::move(transform_ls);

        enforce_waypoint_constraints(index - 1);
        enforce_waypoint_constraints(index);
        enforce_waypoint_constraints(index + 1);
    }

    void Trajectory::set_waypoint_time_from_last(size_t index, float time_from_last)
    {
        m_waypoints[index].m_time_from_last = glm::max(time_from_last, 0.0f);
    }

    void Trajectory::set_waypoint_constraints(size_t index, Waypoint::Constraints constraints)
    {
        auto &old_constraints = m_waypoints[index].m_constraints;

        if (constraints == old_constraints)
        {
            return;
        }

        using namespace magic_enum::bitwise_operators;

        if ((constraints & Waypoint::Constraints::AlignWithPrevious) != Waypoint::Constraints::None && (constraints & Waypoint::Constraints::AlignWithNext) != Waypoint::Constraints::None)
        {
            if ((old_constraints & Waypoint::Constraints::AlignWithPrevious) != Waypoint::Constraints::None)
            {
                constraints &= ~Waypoint::Constraints::AlignWithPrevious;
            }
            else
            {
                constraints &= ~Waypoint::Constraints::AlignWithNext;
            }
        }
        old_constraints = constraints;
        enforce_waypoint_constraints(index);
    }

    bool Trajectory::can_translate_waypoint(size_t) const
    {
        return true;
    }

    bool Trajectory::can_rotate_waypoint(size_t index) const
    {
        using namespace magic_enum::bitwise_operators;

        if ((m_waypoints[index].m_constraints & Waypoint::Constraints::AlignWithPrevious) != Waypoint::Constraints::None && index > 0)
        {
            return false;
        }

        if ((m_waypoints[index].m_constraints & Waypoint::Constraints::AlignWithNext) != Waypoint::Constraints::None && index < m_waypoints.size() - 1)
        {
            return false;
        }
        return true;
    }

    bool Trajectory::can_scale_waypoint(size_t) const
    {
        return false;
    }

    void Trajectory::create_follow_trajectory_client()
    {
        try
        {
            m_follow_trajectory_client = rclcpp_action::create_client<nav2_msgs::action::FollowPath>(m_node, m_action_name);
        }
        catch (const rclcpp::exceptions::InvalidTopicNameError &exception)
        {
            RCLCPP_ERROR(m_node->get_logger(), "Failed to create follow trajectory client: %s", exception.what());
            m_follow_trajectory_client = nullptr;
        }
    }

    void Trajectory::follow_trajectory()
    {
        if (m_waypoints.size() == 0)
        {
            std::lock_guard lock(m_follow_trajectory_mutex);
            m_follow_trajectory_status = "Cannot follow an empty trajectory.";
            RCLCPP_ERROR(m_node->get_logger(), "Cannot follow an empty trajectory.");
            return;
        }

        if (m_path_spacing <= 0.0f)
        {
            std::lock_guard lock(m_follow_trajectory_mutex);
            m_follow_trajectory_status = "Path spacing must be greater than zero.";
            RCLCPP_ERROR(m_node->get_logger(), "Path spacing must be greater than zero.");
            return;
        }

        if (!m_follow_trajectory_client)
        {
            std::lock_guard lock(m_follow_trajectory_mutex);
            m_follow_trajectory_status = "Action client is not initialized. Cannot follow trajectory.";
            RCLCPP_ERROR(m_node->get_logger(), "Action client is not initialized. Cannot follow trajectory.");
            return;
        }

        if (!m_follow_trajectory_client->action_server_is_ready())
        {
            std::lock_guard lock(m_follow_trajectory_mutex);
            m_follow_trajectory_status = std::string("Action server '%s' is not ready.", m_action_name.c_str());
            RCLCPP_ERROR(m_node->get_logger(), "Action server '%s' is not ready.", m_action_name.c_str());
            return;
        }

        auto ls_to_ws_matrix = get_ls_to_ws_matrix();

        auto action = nav2_msgs::action::FollowPath::Goal();
        action.controller_id = "FollowPath";
        auto &path = action.path;
        path.header.frame_id = "map";

        for (size_t index = 0; index < m_waypoints.size(); index++)
        {
            glm::mat4 waypoint_ws_matrix = ls_to_ws_matrix * m_waypoints[index].m_transform_ls.get_matrix();
            glm::vec3 waypoint_position = glm::vec3(waypoint_ws_matrix[3]);
            const glm::vec3 waypoint_forward = glm::vec3(waypoint_ws_matrix[0]);
            const float curr_yaw = std::atan2(waypoint_forward.y, waypoint_forward.x);

            if (index == 0)
            {
                geometry_msgs::msg::PoseStamped pose;
                pose.header.frame_id = "map";
                pose.pose.position.x = waypoint_position.x;
                pose.pose.position.y = waypoint_position.y;
                pose.pose.orientation.w = std::cos(curr_yaw * 0.5f);
                pose.pose.orientation.z = std::sin(curr_yaw * 0.5f);
                path.poses.push_back(pose);
                continue;
            }

            glm::mat4 last_waypoint_ws_matrix = ls_to_ws_matrix * m_waypoints[index - 1].m_transform_ls.get_matrix();
            glm::vec3 last_waypoint_position = glm::vec3(last_waypoint_ws_matrix[3]);
            const glm::vec3 last_waypoint_forward = glm::vec3(last_waypoint_ws_matrix[0]);
            const float last_yaw = std::atan2(last_waypoint_forward.y, last_waypoint_forward.x);
            const float distance = glm::length(waypoint_position - last_waypoint_position);

            if (distance < 1e-4f)
            {
                continue;
            }

            const float delta_yaw = std::atan2(std::sin(curr_yaw - last_yaw), std::cos(curr_yaw - last_yaw));
            const size_t step_count = std::max<size_t>(1, static_cast<size_t>(std::ceil(distance / m_path_spacing)));

            for (size_t step = 1; step <= step_count; step++)
            {
                const float step_amount = static_cast<float>(step) / static_cast<float>(step_count);
                const glm::vec3 interpolated_pos = glm::mix(last_waypoint_position, waypoint_position, step_amount);
                const float interpolated_yaw = last_yaw + delta_yaw * step_amount;

                geometry_msgs::msg::PoseStamped pose;
                pose.header.frame_id = "map";
                pose.pose.position.x = interpolated_pos.x;
                pose.pose.position.y = interpolated_pos.y;
                pose.pose.orientation.z = std::sin(interpolated_yaw * 0.5f);
                pose.pose.orientation.w = std::cos(interpolated_yaw * 0.5f);
                path.poses.push_back(pose);
            }
        }

        auto send_goal_options = rclcpp_action::Client<nav2_msgs::action::FollowPath>::SendGoalOptions();

        send_goal_options.goal_response_callback = [this](rclcpp_action::Client<nav2_msgs::action::FollowPath>::GoalHandle::SharedPtr goal_handle)
        {
            std::lock_guard lock(m_follow_trajectory_mutex);

            if (goal_handle)
            {
                m_follow_trajectory_goal_handle = goal_handle;
                m_follow_trajectory_status = "FollowPath goal accepted.";
            }
            else
            {
                m_follow_trajectory_status = "FollowPath goal rejected.";
                RCLCPP_ERROR(m_node->get_logger(), "%s", m_follow_trajectory_status.c_str());
            }
        };

        send_goal_options.feedback_callback = [this](rclcpp_action::Client<nav2_msgs::action::FollowPath>::GoalHandle::SharedPtr, const std::shared_ptr<const nav2_msgs::action::FollowPath::Feedback> feedback)
        {
            std::lock_guard lock(m_follow_trajectory_mutex);
            m_follow_trajectory_status = "Following path: %.2f m remaining", feedback->distance_to_goal;
        };

        send_goal_options.result_callback = [this](const rclcpp_action::ClientGoalHandle<nav2_msgs::action::FollowPath>::WrappedResult &result)
        {
            std::lock_guard lock(m_follow_trajectory_mutex);
            m_follow_trajectory_goal_handle.reset();

            if (result.result)
            {
                m_follow_trajectory_status = "FollowPath finished.";

                if (!result.result->error_msg.empty())
                {
                    RCLCPP_ERROR(m_node->get_logger(), "%s", result.result->error_msg.c_str());
                }
            }
            else
            {
                m_follow_trajectory_status = "FollowPath finished without a result.";
            }
        };

        {
            std::lock_guard lock(m_follow_trajectory_mutex);
            m_follow_trajectory_status = "Sending FollowPath goal...";
        }
        m_follow_trajectory_client->async_send_goal(action, send_goal_options);
    }

    void Trajectory::cancel_follow_trajectory()
    {
        std::lock_guard lock(m_follow_trajectory_mutex);

        if (!m_follow_trajectory_goal_handle)
        {
            m_follow_trajectory_status = "No active FollowPath goal.";
            return;
        }

        m_follow_trajectory_client->async_cancel_goal(m_follow_trajectory_goal_handle);
        m_follow_trajectory_status = "Cancelling FollowPath goal...";
    }

    bool Trajectory::follow_trajectory_active() const
    {
        std::lock_guard lock(m_follow_trajectory_mutex);
        return m_follow_trajectory_goal_handle != nullptr;
    }

    std::string Trajectory::get_follow_trajectory_status() const
    {
        std::lock_guard lock(m_follow_trajectory_mutex);
        return m_follow_trajectory_status;
    }

    void Trajectory::enforce_waypoint_constraints(size_t index)
    {
        if (index >= m_waypoints.size())
        {
            return;
        }

        using namespace magic_enum::bitwise_operators;

        if ((m_waypoints[index].m_constraints & Waypoint::Constraints::AlignWithPrevious) != Waypoint::Constraints::None && index > 0)
        {
            glm::vec3 direction_ls = m_waypoints[index].m_transform_ls.get_translation() - m_waypoints[index - 1].m_transform_ls.get_translation();
            m_waypoints[index].m_transform_ls.look_in(direction_ls);
        }

        if ((m_waypoints[index].m_constraints & Waypoint::Constraints::AlignWithNext) != Waypoint::Constraints::None && index < m_waypoints.size() - 1)
        {
            glm::vec3 direction_ls = m_waypoints[index + 1].m_transform_ls.get_translation() - m_waypoints[index].m_transform_ls.get_translation();
            m_waypoints[index].m_transform_ls.look_in(direction_ls);
        }
    }
}