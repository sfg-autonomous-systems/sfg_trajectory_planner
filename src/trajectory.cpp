#include "sfg_trajectory_planner/trajectory.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include "sfg_imgui_vendor/push_id_guard.hpp"
#include "sfg_trajectory_planner/core/gfx/renderer.hpp"
#include "sfg_trajectory_planner/core/gfx/utils.hpp"
#include "sfg_trajectory_planner/core/scene.hpp"
#include "sfg_trajectory_planner/editor/selection_context.hpp"

namespace sfg_trajectory_planner
{
    static constexpr auto s_waypoints_table_flags = ImGuiTableFlags_BordersV |
                                                    ImGuiTableFlags_BordersOuterH |
                                                    ImGuiTableFlags_RowBg |
                                                    ImGuiTableFlags_SizingFixedFit;

    static constexpr auto s_add_button_text = "+";
    static constexpr auto s_remove_button_text = "-";

    Trajectory::Trajectory(core::SceneObject::ConstructionKey key, core::Scene &scene, uuids::uuid uuid, const core::gfx::Camera &camera, editor::SelectionContext &selection_context)
        : SceneObject(key, scene, uuid),
          m_camera(camera),
          m_selection_context(selection_context)
    {
    }

    void Trajectory::render_object(core::gfx::Renderer &renderer)
    {
        auto object_to_world_matrix = get_object_to_world_matrix();

        for (size_t index = 1; index < m_waypoints.size(); index++)
        {
            glm::vec3 start = glm::vec3(object_to_world_matrix * glm::vec4(m_waypoints[index - 1].m_transform.get_translation(), 1.0f));
            glm::vec3 end = glm::vec3(object_to_world_matrix * glm::vec4(m_waypoints[index].m_transform.get_translation(), 1.0f));
            renderer.add_line(start, end, m_color);
        }

        if (m_selection_context.get_selected() != this)
        {
            return;
        }

        auto world_to_object_matrix = get_world_to_object_matrix();
        auto gizmo_operation = m_selection_context.get_gizmo_operation();

        if (m_selected_waypoint_index != -1 && gizmo_operation != ImGuizmo::SCALE)
        {
            auto &waypoint = m_waypoints[m_selected_waypoint_index];
            auto waypoint_object_to_world_matrix = object_to_world_matrix * waypoint.m_transform.get_matrix();

            if (renderer.add_gizmo(waypoint_object_to_world_matrix, gizmo_operation, m_selection_context.get_gizmo_mode(), &waypoint))
            {
                waypoint.m_transform = world_to_object_matrix * waypoint_object_to_world_matrix;
            }
        }

        for (size_t index = 0; index < m_waypoints.size(); index++)
        {
            glm::vec3 waypoint_position = glm::vec3(object_to_world_matrix * glm::vec4(m_waypoints[index].m_transform.get_translation(), 1.0f));
            auto screen_position = m_camera.world_to_screen_point(waypoint_position);
            auto can_select_waypoint = !ImGuizmo::IsOver() && !ImGuizmo::IsUsingAny();
            auto is_hovering_waypoint = glm::length(screen_position.xy() - glm::vec2(ImGui::GetMousePos().x, ImGui::GetMousePos().y)) < 10.0f && ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);

            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && can_select_waypoint && is_hovering_waypoint)
            {
                m_selected_waypoint_index = static_cast<std::int32_t>(index);
            }
            renderer.add_text(waypoint_position, std::to_string(index));
        }
    }

    void Trajectory::render_inspector_internal()
    {
        ImGui::InputText("Topic Name", &m_topic_name);
        ImGui::InputText("Frame ID", &m_frame_id);

        if (ImGui::InputFloat("Time from start [s]", &m_time_from_start))
        {
            m_time_from_start = std::max(0.0f, m_time_from_start);
        }

        ImGui::ColorEdit3("Color", glm::value_ptr(m_color));

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
                auto waypoint = Waypoint();
                // The new waypoint's location should be one unit forward from the last waypoint or from the origin if there are no waypoints yet.
                auto last_transform = m_waypoints.empty() ? glm::mat4(1.0f) : m_waypoints.back().m_transform.get_matrix();
                waypoint.m_transform = core::Transform(last_transform * glm::translate(glm::mat4(1.0f), core::gfx::utils::s_forward.xyz()));
                m_waypoints.push_back(waypoint);
            }

            for (size_t index = 0; index < m_waypoints.size(); index++)
            {
                sfg_imgui_vendor::PushIdGuard id_guard(index);
                auto &waypoint = m_waypoints[index];

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
                }

                ImGui::TableNextColumn();
                waypoint.m_transform.render_inspector(true, true, false, false);

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
                    m_waypoints.erase(m_waypoints.begin() + index--);
                }
            }
            ImGui::EndTable();
        }
    }
}