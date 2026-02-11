#include "sfg_trajectory_planner/trajectory.hpp"

#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include "sfg_imgui_vendor/push_id_guard.hpp"
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

    Trajectory::Trajectory(core::SceneObjectKey key, core::Scene &scene, uuids::uuid uuid, editor::SelectionContext &selection_context)
        : SceneObject(key, scene, uuid),
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
            renderer.add_line(start, end, glm::vec3(1.0f, 1.0f, 0.0f));
        }

        if (m_selection_context.get_selected() != this)
        {
            return;
        }

        auto world_to_object_matrix = get_world_to_object_matrix();
        auto gizmo_operation = m_selection_context.get_gizmo_operation();

        // Scaling the trajectory doesn't make much sense, so we disable the scale gizmo.
        if (gizmo_operation == ImGuizmo::SCALE)
        {
            return;
        }

        for (size_t index = 0; index < m_waypoints.size(); index++)
        {
            auto &waypoint = m_waypoints[index];
            auto waypoint_object_to_world_matrix = object_to_world_matrix * waypoint.m_transform.get_matrix();

            if (renderer.add_gizmo(waypoint_object_to_world_matrix, gizmo_operation, m_selection_context.get_gizmo_mode(), &waypoint))
            {
                waypoint.m_transform = world_to_object_matrix * waypoint_object_to_world_matrix;
            }
            renderer.add_text(waypoint.m_transform.get_translation(), std::to_string(index));
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
                ImGui::SameLine();
                ImGui::Text("Waypoint %zu", index);

                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0f);

                if (ImGui::InputFloat("##time_from_last", &waypoint.m_time_from_last))
                {
                    waypoint.m_time_from_last = std::max(0.0f, waypoint.m_time_from_last);
                }

                ImGui::TableNextColumn();
                waypoint.m_transform.render_inspector(true, true, false);

                ImGui::TableNextColumn();

                if (ImGui::Button(s_remove_button_text))
                {
                    m_waypoints.erase(m_waypoints.begin() + index--);
                }
            }
            ImGui::EndTable();
        }
    }
}