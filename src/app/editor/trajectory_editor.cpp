#include "sfg_trajectory_planner/app/editor/trajectory_editor.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include "sfg_imgui_vendor/push_id_guard.hpp"
#include "sfg_trajectory_planner/app/core/trajectory.hpp"
#include "sfg_trajectory_planner/engine/core/gfx/camera.hpp"
#include "sfg_trajectory_planner/engine/core/gfx/renderer.hpp"
#include "sfg_trajectory_planner/engine/core/gfx/utils.hpp"
#include "sfg_trajectory_planner/engine/editor/selection_context.hpp"

namespace sfg_trajectory_planner::app::editor
{
    static constexpr auto s_waypoints_table_flags = ImGuiTableFlags_BordersV |
                                                    ImGuiTableFlags_BordersOuterH |
                                                    ImGuiTableFlags_RowBg |
                                                    ImGuiTableFlags_SizingFixedFit;

    static constexpr auto s_add_button_text = "+";
    static constexpr auto s_remove_button_text = "-";
    static constexpr auto s_modify_waypoint_constraints_popup_id = "modify_waypoint_constraints_popup";
    static constexpr auto s_modify_waypoint_constraints_button_text = "C";

    TrajectoryEditor::TrajectoryEditor(const engine::editor::SelectionContext &selection_context, rclcpp::Node *node) : SceneObjectEditor(selection_context), m_node(node)
    {
        create_trajectory_publisher(dynamic_cast<core::Trajectory *>(m_selection_context.get_selected())->get_topic_name());
    }

    bool TrajectoryEditor::render_editor(engine::core::gfx::Renderer &renderer)
    {
        auto changed = SceneObjectEditor::render_editor(renderer);
        auto trajectory = dynamic_cast<core::Trajectory *>(m_selection_context.get_selected());
        auto object_to_world_matrix = trajectory->get_object_to_world_matrix();
        auto gizmo_operation = m_selection_context.get_gizmo_operation();
        auto &waypoints = trajectory->get_waypoints();

        if (m_selected_waypoint_index < waypoints.size() &&
            ((gizmo_operation == ImGuizmo::TRANSLATE && can_translate_waypoint(m_selected_waypoint_index)) ||
             (gizmo_operation == ImGuizmo::ROTATE && can_rotate_waypoint(m_selected_waypoint_index))))
        {
            auto &waypoint = waypoints[m_selected_waypoint_index];
            changed |= render_transform_editor(renderer, waypoint.m_transform, object_to_world_matrix);
        }

        for (size_t index = 0; index < waypoints.size(); index++)
        {
            glm::vec3 waypoint_position = glm::vec3(object_to_world_matrix * glm::vec4(waypoints[index].m_transform.get_translation(), 1.0f));
            auto screen_position = renderer.get_camera().world_to_screen_point(waypoint_position);
            auto can_select_waypoint = !ImGuizmo::IsOver() && !ImGuizmo::IsUsingAny();
            auto is_hovering_waypoint = glm::length(screen_position.xy() - glm::vec2(ImGui::GetMousePos().x, ImGui::GetMousePos().y)) < 10.0f && ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);

            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && can_select_waypoint && is_hovering_waypoint)
            {
                m_selected_waypoint_index = index;
            }
            renderer.add_text(waypoint_position, std::to_string(index));
        }
        return changed;
    }

    bool TrajectoryEditor::render_inspector()
    {
        auto changed = SceneObjectEditor::render_inspector();

        if (!ImGui::CollapsingHeader("Object", ImGuiTreeNodeFlags_DefaultOpen))
        {
            return changed;
        }

        if (ImGui::Button("Publish Trajectory", ImVec2(-1.0f, 0.0f)))
        {
            publish_trajectory(m_node->now());
        }

        auto trajectory = dynamic_cast<core::Trajectory *>(m_selection_context.get_selected());
        auto topic_name = trajectory->get_topic_name();

        if (ImGui::InputText("Topic Name", &topic_name))
        {
            trajectory->set_topic_name(topic_name);
            changed = true;
        }

        if (ImGui::IsItemDeactivatedAfterEdit())
        {
            create_trajectory_publisher(trajectory->get_topic_name());
        }

        auto frame_id = trajectory->get_frame_id();

        if (ImGui::InputText("Frame ID", &frame_id))
        {
            trajectory->set_frame_id(frame_id);
            changed = true;
        }

        auto time_from_start = trajectory->get_time_from_start();

        if (ImGui::InputFloat("Time from start [s]", &time_from_start))
        {
            trajectory->set_time_from_start(time_from_start);
            changed = true;
        }

        auto color = trajectory->get_color();

        if (ImGui::ColorEdit3("Color", glm::value_ptr(color)))
        {
            trajectory->set_color(color);
            changed = true;
        }

        auto &waypoints = trajectory->get_waypoints();

        if (ImGui::BeginTable("waypoints_table", 4, s_waypoints_table_flags))
        {
            ImGui::TableSetupColumn("Waypoint", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Time from last [s]", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Transform", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_IndentDisable, ImGui::CalcTextSize(s_add_button_text).x + ImGui::GetStyle().ItemSpacing.x);

            ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
            ImGui::TableNextColumn();
            ImGui::TableHeader("Waypoint");

            ImGui::TableNextColumn();
            ImGui::TableHeader("Time from last [s]");

            ImGui::TableNextColumn();
            ImGui::TableHeader("Transform");

            ImGui::TableNextColumn();

            if (ImGui::Button(s_add_button_text))
            {
                auto waypoint = core::Waypoint();
                // The new waypoint's location should be one unit forward from the last waypoint or from the origin if there are no waypoints yet.
                auto last_transform = waypoints.empty() ? glm::mat4(1.0f) : waypoints.back().m_transform.get_matrix();
                // The new waypoint should inherit the constraints of the last waypoint or have no constraints if there are no waypoints yet.
                auto last_constraints = waypoints.empty() ? core::Waypoint::Constraints::None : waypoints.back().m_constraints;

                waypoint.m_transform = engine::core::Transform(last_transform * glm::translate(glm::mat4(1.0f), engine::core::gfx::utils::s_forward.xyz()));
                waypoint.m_constraints = last_constraints;
                waypoints.push_back(waypoint);
                changed = true;
            }
            ImGui::SetItemTooltip("Add Waypoint");

            for (size_t index = 0; index < waypoints.size(); index++)
            {
                sfg_imgui_vendor::PushIdGuard id_guard(index);
                auto &waypoint = waypoints[index];

                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                if (ImGui::Selectable("##waypoint_selectable", m_selected_waypoint_index == index))
                {
                    m_selected_waypoint_index = index;
                }

                ImGui::SameLine();
                ImGui::Text("Waypoint %zu", index);

                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0f);

                if (ImGui::InputFloat("##time_from_last", &waypoint.m_time_from_last))
                {
                    waypoint.m_time_from_last = std::max(0.0f, waypoint.m_time_from_last);
                    changed = true;
                }

                ImGui::TableNextColumn();
                changed |= render_transform_inspector(waypoint.m_transform, can_translate_waypoint(index), can_rotate_waypoint(index), false, false);

                ImGui::TableNextColumn();

                if (ImGui::Button(s_remove_button_text))
                {
                    if (m_selected_waypoint_index == index)
                    {
                        m_selected_waypoint_index = std::numeric_limits<size_t>::max();
                    }
                    else if (m_selected_waypoint_index > index)
                    {
                        m_selected_waypoint_index--;
                    }
                    waypoints.erase(waypoints.begin() + index--);
                    changed = true;
                }
                ImGui::SetItemTooltip("Remove Waypoint");

                if (ImGui::Button(s_modify_waypoint_constraints_button_text))
                {
                    ImGui::OpenPopup(s_modify_waypoint_constraints_popup_id);
                }
                ImGui::SetItemTooltip("Modify Waypoint Constraints");

                if (ImGui::BeginPopup(s_modify_waypoint_constraints_popup_id))
                {
                    using namespace magic_enum::bitwise_operators;

                    for (auto constraint : magic_enum::enum_values<core::Waypoint::Constraints>())
                    {
                        bool selected = (waypoint.m_constraints & constraint) != core::Waypoint::Constraints::None;

                        if (ImGui::Selectable(magic_enum::enum_name(constraint).data(), selected, ImGuiSelectableFlags_DontClosePopups))
                        {
                            if (selected)
                            {
                                waypoint.m_constraints = waypoint.m_constraints & ~constraint;
                            }
                            else
                            {
                                waypoint.m_constraints = waypoint.m_constraints | constraint;
                            }
                        }
                    }
                    ImGui::EndPopup();
                }
            }
            ImGui::EndTable();
        }
        return changed;
    }

    void TrajectoryEditor::publish_trajectory(rclcpp::Time time)
    {
        if (!m_trajectory_publisher)
        {
            return;
        }

        auto trajectory = dynamic_cast<core::Trajectory *>(m_selection_context.get_selected());
        auto object_to_world_matrix = trajectory->get_object_to_world_matrix();

        auto msg = std::make_unique<sfg_agent_msgs::msg::Trajectory>();
        msg->header.stamp = time + rclcpp::Duration::from_seconds(trajectory->get_time_from_start());
        msg->header.frame_id = trajectory->get_frame_id();

        for (const auto &waypoint : trajectory->get_waypoints())
        {
            glm::mat4 waypoint_matrix = object_to_world_matrix * waypoint.m_transform.get_matrix();
            glm::vec3 waypoint_position = glm::vec3(waypoint_matrix[3]);
            glm::quat waypoint_rotation = glm::quat_cast(waypoint_matrix);

            sfg_agent_msgs::msg::Waypoint waypoint_msg;
            waypoint_msg.pose.position.x = waypoint_position.x;
            waypoint_msg.pose.position.y = waypoint_position.y;
            waypoint_msg.pose.position.z = waypoint_position.z;
            waypoint_msg.pose.orientation.x = waypoint_rotation.x;
            waypoint_msg.pose.orientation.y = waypoint_rotation.y;
            waypoint_msg.pose.orientation.z = waypoint_rotation.z;
            waypoint_msg.pose.orientation.w = waypoint_rotation.w;
            waypoint_msg.time_from_last = rclcpp::Duration::from_seconds(waypoint.m_time_from_last);
            msg->waypoints.push_back(waypoint_msg);
        }
        m_trajectory_publisher->publish(*msg);
    }

    void TrajectoryEditor::create_trajectory_publisher(const std::string &topic_name)
    {
        try
        {
            m_trajectory_publisher = m_node->template create_publisher<sfg_agent_msgs::msg::Trajectory>(topic_name, 10);
        }
        catch (const rclcpp::exceptions::InvalidTopicNameError &exception)
        {
            RCLCPP_ERROR(m_node->get_logger(), "Failed to create trajectory publisher: %s", exception.what());
            m_trajectory_publisher = nullptr;
        }
    }

    bool TrajectoryEditor::can_translate_waypoint(size_t)
    {
        return true;
    }

    bool TrajectoryEditor::can_rotate_waypoint(size_t index)
    {
        using namespace magic_enum::bitwise_operators;

        const auto trajectory = dynamic_cast<core::Trajectory *>(m_selection_context.get_selected());
        const auto &waypoint = trajectory->get_waypoints()[index];

        if ((waypoint.m_constraints & core::Waypoint::Constraints::AlignWithPrevious) != core::Waypoint::Constraints::None && index != 0)
        {
            return false;
        }

        if ((waypoint.m_constraints & core::Waypoint::Constraints::AlignWithNext) != core::Waypoint::Constraints::None && index != trajectory->get_waypoints().size() - 1)
        {
            return false;
        }
        return true;
    }

    void TrajectoryEditor::apply_waypoint_constraints(size_t index)
    {
        using namespace magic_enum::bitwise_operators;

        const auto trajectory = dynamic_cast<core::Trajectory *>(m_selection_context.get_selected());
        auto &waypoint = trajectory->get_waypoints()[index];

        if ((waypoint.m_constraints & core::Waypoint::Constraints::AlignWithPrevious) != core::Waypoint::Constraints::None && index != 0)
        {
            auto &previous_waypoint = trajectory->get_waypoints()[index - 1];
            // ToDo: Implement this.
        }

        if ((waypoint.m_constraints & core::Waypoint::Constraints::AlignWithNext) != core::Waypoint::Constraints::None && index != trajectory->get_waypoints().size() - 1)
        {
            auto &next_waypoint = trajectory->get_waypoints()[index + 1];
            // ToDo: Implement this.
        }
    }
}