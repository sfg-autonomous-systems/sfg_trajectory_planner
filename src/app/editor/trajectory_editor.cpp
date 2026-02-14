#include "sfg_trajectory_planner/app/editor/trajectory_editor.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include "sfg_imgui_vendor/push_id_guard.hpp"
#include "sfg_trajectory_planner/engine/core/gfx/utils.hpp"
#include "sfg_trajectory_planner/app/core/trajectory.hpp"

namespace sfg_trajectory_planner::app::editor
{
    static constexpr auto s_waypoints_table_flags = ImGuiTableFlags_BordersV |
                                                    ImGuiTableFlags_BordersOuterH |
                                                    ImGuiTableFlags_RowBg |
                                                    ImGuiTableFlags_SizingFixedFit;

    static constexpr auto s_add_button_text = "+";
    static constexpr auto s_remove_button_text = "-";

    TrajectoryEditor::TrajectoryEditor(engine::editor::SelectionContext &selection_context) : SceneObjectEditor(selection_context) {}

    bool TrajectoryEditor::render_editor(engine::core::gfx::Renderer &renderer)
    {
        auto changed = false;
        auto trajectory = dynamic_cast<core::Trajectory *>(m_selection_context.get_selected());
        auto world_to_object_matrix = trajectory->get_world_to_object_matrix();
        auto object_to_world_matrix = trajectory->get_object_to_world_matrix();
        auto gizmo_operation = m_selection_context.get_gizmo_operation();
        auto &waypoints = trajectory->get_waypoints();

        if (m_selected_waypoint_index != -1 && gizmo_operation != ImGuizmo::SCALE)
        {
            auto &waypoint = waypoints[m_selected_waypoint_index];
            auto waypoint_object_to_world_matrix = object_to_world_matrix * waypoint.m_transform.get_matrix();

            if (renderer.add_gizmo(waypoint_object_to_world_matrix, gizmo_operation, m_selection_context.get_gizmo_mode(), &waypoint))
            {
                waypoint.m_transform = world_to_object_matrix * waypoint_object_to_world_matrix;
                changed = true;
            }
        }

        for (size_t index = 0; index < waypoints.size(); index++)
        {
            glm::vec3 waypoint_position = glm::vec3(object_to_world_matrix * glm::vec4(waypoints[index].m_transform.get_translation(), 1.0f));
            auto screen_position = renderer.get_camera().world_to_screen_point(waypoint_position);
            auto can_select_waypoint = !ImGuizmo::IsOver() && !ImGuizmo::IsUsingAny();
            auto is_hovering_waypoint = glm::length(screen_position.xy() - glm::vec2(ImGui::GetMousePos().x, ImGui::GetMousePos().y)) < 10.0f && ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);

            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && can_select_waypoint && is_hovering_waypoint)
            {
                m_selected_waypoint_index = static_cast<std::int32_t>(index);
            }
            renderer.add_text(waypoint_position, std::to_string(index));
        }
        return changed;
    }

    bool TrajectoryEditor::render_inspector()
    {
        auto changed = SceneObjectEditor::render_inspector();

        if (ImGui::CollapsingHeader("Object", ImGuiTreeNodeFlags_DefaultOpen))
        {
            return changed;
        }

        auto trajectory = dynamic_cast<core::Trajectory *>(m_selection_context.get_selected());
        auto topic_name = trajectory->get_topic_name();

        if (ImGui::InputText("Topic Name", &topic_name))
        {
            trajectory->set_topic_name(topic_name);
            changed = true;
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
                auto waypoint = core::Trajectory::Waypoint();
                // The new waypoint's location should be one unit forward from the last waypoint or from the origin if there are no waypoints yet.
                auto last_transform = waypoints.empty() ? glm::mat4(1.0f) : waypoints.back().m_transform.get_matrix();
                waypoint.m_transform = engine::core::Transform(last_transform * glm::translate(glm::mat4(1.0f), engine::core::gfx::utils::s_forward.xyz()));
                waypoints.push_back(waypoint);
                changed = true;
            }

            for (size_t index = 0; index < waypoints.size(); index++)
            {
                sfg_imgui_vendor::PushIdGuard id_guard(index);
                auto &waypoint = waypoints[index];

                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                if (ImGui::Selectable("##waypoint_selectable", m_selected_waypoint_index == static_cast<std::int32_t>(index)))
                {
                    m_selected_waypoint_index = static_cast<std::int32_t>(index);
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
                changed |= render_transform_inspector(waypoint.m_transform, true, true, false, false);

                ImGui::TableNextColumn();

                if (ImGui::Button(s_remove_button_text))
                {
                    if (m_selected_waypoint_index == static_cast<std::int32_t>(index))
                    {
                        m_selected_waypoint_index = -1;
                    }
                    else if (m_selected_waypoint_index > static_cast<std::int32_t>(index))
                    {
                        m_selected_waypoint_index--;
                    }
                    waypoints.erase(waypoints.begin() + index--);
                    changed = true;
                }
            }
            ImGui::EndTable();
        }
        return changed;
    }
}