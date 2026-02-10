#include "sfg_trajectory_planner/trajectory.hpp"

#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include "sfg_trajectory_planner/core/scene.hpp"

namespace sfg_trajectory_planner
{
    static constexpr auto s_waypoints_table_flags = ImGuiTableFlags_BordersV |
                                                    ImGuiTableFlags_BordersOuterH |
                                                    ImGuiTableFlags_RowBg |
                                                    ImGuiTableFlags_SizingFixedFit;

    Trajectory::Trajectory(core::SceneObjectKey key, core::Scene &scene)
        : SceneObject(key, scene),
          m_time_from_start(0.0f)
    {
    }

    void Trajectory::render_object(core::gfx::Renderer &renderer)
    {
        for (size_t child_index = 1; child_index < m_children.size(); child_index++)
        {
            renderer.add_line(
                glm::vec3(m_children[child_index - 1]->get_global_transform()[3]),
                glm::vec3(m_children[child_index]->get_global_transform()[3]),
                glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
        }
    }

    void Trajectory::render_inspector_internal()
    {
        if (ImGui::Button("Add waypoint", ImVec2(-1.0f, 0.0f)))
        {
            m_scene.create_object<core::SceneObject>("Waypoint", this);
        }

        ImGui::AlignTextToFramePadding();
        ImGui::Text("Topic Name:         ");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputText("##topic_name", &m_topic_name);

        ImGui::AlignTextToFramePadding();
        ImGui::Text("Frame ID:           ");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputText("##frame_id", &m_frame_id);

        ImGui::AlignTextToFramePadding();
        ImGui::Text("Time from start [s]:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

        if (ImGui::InputFloat("##time_from_start", &m_time_from_start))
        {
            m_time_from_start = std::max(0.0f, m_time_from_start);
        }

        auto child_count = m_children.size();

        if (m_times_from_last.size() != child_count)
        {
            m_times_from_last.resize(child_count);
        }

        if (child_count == 0)
        {
            return;
        }

        ImGui::BeginTable("waypoints_table", 2, s_waypoints_table_flags);
        ImGui::TableSetupColumn("Waypoint", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Time from last [s]", ImGuiTableColumnFlags_WidthFixed, 150.0f);
        ImGui::TableHeadersRow();

        for (size_t index = 0; index < child_count; index++)
        {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", m_children[index]->get_name().c_str());
            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(-1.0f);

            if (ImGui::InputFloat(("##time_from_last" + std::to_string(index)).c_str(), &m_times_from_last[index]))
            {
                m_times_from_last[index] = std::max(0.0f, m_times_from_last[index]);
            }
        }
        ImGui::EndTable();
    }
}