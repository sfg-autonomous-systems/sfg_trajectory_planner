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

    Trajectory::Trajectory(core::SceneObjectKey key, core::Scene &scene, uuids::uuid uuid, editor::SelectionContext &selection_context)
        : SceneObject(key, scene, uuid),
          m_selection_context(selection_context)
    {
    }

    void Trajectory::render_object(core::gfx::Renderer &renderer)
    {
        auto transform = get_global_transform();

        for (size_t index = 1; index < m_waypoints.size(); index++)
        {
            glm::vec3 start = glm::vec3(transform * m_waypoints[index - 1].m_transform[3]);
            glm::vec3 end = glm::vec3(transform * m_waypoints[index].m_transform[3]);
            renderer.add_line(start, end, glm::vec3(1.0f, 1.0f, 0.0f));
        }

        if (m_selection_context.get_selected() != this)
        {
            return;
        }

        for (auto &waypoint : m_waypoints)
        {
            auto waypoint_transform = transform * waypoint.m_transform;

            if (renderer.add_gizmo(waypoint_transform, m_selection_context.get_gizmo_operation(), m_selection_context.get_gizmo_mode()))
            {
                waypoint.m_transform = glm::inverse(transform) * waypoint_transform;
            }
        }
    }

    void Trajectory::render_inspector_internal()
    {
        if (ImGui::Button("Add waypoint", ImVec2(-1.0f, 0.0f)))
        {
            auto waypoint = Waypoint();

            // The new waypoint's location should be one unit forward from the last waypoint, or from the origin if there are no waypoints yet.
            auto last_transform = m_waypoints.empty() ? glm::mat4(1.0f) : m_waypoints.back().m_transform;
            waypoint.m_transform = last_transform * glm::translate(glm::mat4(1.0f), core::gfx::utils::s_forward.xyz());
            m_waypoints.push_back(waypoint);
        }

        ImGui::InputText("Topic Name", &m_topic_name);
        ImGui::InputText("Frame ID", &m_frame_id);

        if (ImGui::InputFloat("Time from start [s]", &m_time_from_start))
        {
            m_time_from_start = std::max(0.0f, m_time_from_start);
        }

        ImGui::BeginTable("waypoints_table", 2, s_waypoints_table_flags);
        ImGui::TableSetupColumn("Waypoint", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Time from last [s]", ImGuiTableColumnFlags_WidthFixed, 150.0f);
        ImGui::TableHeadersRow();

        for (size_t index = 0; index < m_waypoints.size(); index++)
        {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Waypoint %zu", index);
            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(-1.0f);

            sfg_imgui_vendor::PushIdGuard id_guard(index);

            if (ImGui::InputFloat("##time_from_last", &m_waypoints[index].m_time_from_last))
            {
                m_waypoints[index].m_time_from_last = std::max(0.0f, m_waypoints[index].m_time_from_last);
            }
        }
        ImGui::EndTable();
    }
}