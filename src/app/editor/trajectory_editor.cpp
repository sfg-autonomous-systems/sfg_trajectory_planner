#include "sfg_trajectory_planner/app/editor/trajectory_editor.hpp"

#include <fmt/format.h>
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
        auto object_to_world_matrix = trajectory->get_ls_to_ws_matrix();
        auto gizmo_operation = m_selection_context.get_gizmo_operation();

        if (m_selected_waypoint_index < trajectory->get_waypoint_count())
        {
            if ((gizmo_operation == ImGuizmo::OPERATION::TRANSLATE && trajectory->can_translate_waypoint(m_selected_waypoint_index)) ||
                (gizmo_operation == ImGuizmo::OPERATION::ROTATE && trajectory->can_rotate_waypoint(m_selected_waypoint_index)) ||
                (gizmo_operation == ImGuizmo::OPERATION::SCALE && trajectory->can_scale_waypoint(m_selected_waypoint_index)))
            {
                const auto &transform = trajectory->get_waypoint_transform(m_selected_waypoint_index);

                if (auto modified_transform = render_transform_editor(renderer, transform, object_to_world_matrix))
                {
                    trajectory->set_waypoint_transform(m_selected_waypoint_index, modified_transform.value());
                    changed = true;
                }
            }
        }

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            auto did_select_waypoint = false;

            for (size_t index = 0; index < trajectory->get_waypoint_count(); index++)
            {
                glm::vec3 waypoint_position = glm::vec3(object_to_world_matrix * glm::vec4(trajectory->get_waypoint_transform(index).get_translation(), 1.0f));
                auto screen_position = renderer.get_camera().world_to_screen_point(waypoint_position);
                auto can_select_waypoint = !ImGuizmo::IsOver() && !ImGuizmo::IsUsingAny();
                auto is_hovering_waypoint = glm::length(screen_position.xy() - glm::vec2(ImGui::GetMousePos().x, ImGui::GetMousePos().y)) < 10.0f && ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);

                if (can_select_waypoint && is_hovering_waypoint)
                {
                    m_selected_waypoint_index = index;
                    did_select_waypoint = true;
                    break;
                }
            }

            if (!did_select_waypoint && !ImGuizmo::IsUsingAny())
            {
                m_selected_waypoint_index = std::numeric_limits<size_t>::max();
            }
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

        if (ImGui::DragFloat("Time from start [s]", &time_from_start, 0.01f, 0.0f, std::numeric_limits<float>::max()))
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
                trajectory->add_waypoint();
                changed = true;
            }
            ImGui::SetItemTooltip("Add Waypoint");

            for (size_t index = 0; index < trajectory->get_waypoint_count(); index++)
            {
                sfg_imgui_vendor::PushIdGuard id_guard(index);

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

                auto time_from_last = trajectory->get_waypoint_time_from_last(index);

                if (ImGui::DragFloat("##time_from_last", &time_from_last, 0.01f, 0.0f, std::numeric_limits<float>::max()))
                {
                    trajectory->set_waypoint_time_from_last(index, time_from_last);
                    changed = true;
                }

                ImGui::TableNextColumn();
                auto transform = trajectory->get_waypoint_transform(index);

                if (render_transform_inspector(transform, trajectory->can_translate_waypoint(index), trajectory->can_rotate_waypoint(index), trajectory->can_scale_waypoint(index), false))
                {
                    trajectory->set_waypoint_transform(index, transform);
                    changed = true;
                }

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
                    trajectory->remove_waypoint(index);
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
                    auto constraints = trajectory->get_waypoint_constraints(index);

                    for (auto constraint : magic_enum::enum_values<core::Waypoint::Constraints>())
                    {
                        bool selected = (constraints & constraint) != core::Waypoint::Constraints::None;

                        if (ImGui::Selectable(magic_enum::enum_name(constraint).data(), selected, ImGuiSelectableFlags_DontClosePopups))
                        {
                            if (selected)
                            {
                                trajectory->set_waypoint_constraints(index, trajectory->get_waypoint_constraints(index) & ~constraint);
                            }
                            else
                            {
                                trajectory->set_waypoint_constraints(index, trajectory->get_waypoint_constraints(index) | constraint);
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
        auto object_to_world_matrix = trajectory->get_ls_to_ws_matrix();

        auto msg = std::make_unique<sfg_agent_msgs::msg::Trajectory>();
        msg->header.stamp = time + rclcpp::Duration::from_seconds(trajectory->get_time_from_start());
        msg->header.frame_id = trajectory->get_frame_id();

        for (size_t index = 0; index < trajectory->get_waypoint_count(); index++)
        {
            glm::mat4 waypoint_matrix = object_to_world_matrix * trajectory->get_waypoint_transform(index).get_matrix();
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
            waypoint_msg.time_from_last = rclcpp::Duration::from_seconds(trajectory->get_waypoint_time_from_last(index));
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
}