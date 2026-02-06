#include "sfg_trajectory_planner/trajectories_inspector.hpp"

#include <sfg_imgui_vendor/push_id_guard.hpp>

namespace sfg_trajectory_planner
{
    static constexpr auto s_trajectories_table_flags = ImGuiTableFlags_BordersV |
                                                       ImGuiTableFlags_BordersOuterH |
                                                       ImGuiTableFlags_RowBg |
                                                       ImGuiTableFlags_SizingFixedFit;
    static constexpr auto s_add_button_text = "+";
    static constexpr auto s_remove_button_text = "-";
    static constexpr auto s_move_up_button_text = "^";
    static constexpr auto s_move_down_button_text = "v";

    TrajectoriesInspector::TrajectoriesInspector(std::vector<Trajectory> &trajectories)
        : GuiElement(),
          m_trajectories(trajectories)
    {
    }

    void TrajectoriesInspector::render_internal()
    {
        if (!ImGui::CollapsingHeader("Trajectories", ImGuiTreeNodeFlags_DefaultOpen))
        {
            return;
        }

        if (ImGui::BeginTable("trajectories_table", 4, s_trajectories_table_flags))
        {
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, ImGui::CalcTextSize(s_add_button_text).x + ImGui::CalcTextSize(s_remove_button_text).x * 3 + ImGui::GetStyle().ItemSpacing.x);

            ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
            ImGui::TableNextColumn();
            ImGui::TableHeader("Trajectory");

            ImGui::TableNextColumn();
            ImGui::TableHeader("Waypoint");
            ImGui::TableNextColumn();
            ImGui::TableHeader("Delay from... [s]");
            ImGui::TableNextColumn();

            if (ImGui::Button(s_add_button_text))
            {
                m_trajectories.emplace_back();
            }
            ImGui::SameLine();

            if (ImGui::Button(s_remove_button_text))
            {
            }

            for (size_t trajectory_index = 0; trajectory_index < m_trajectories.size(); trajectory_index++)
            {
                render_trajectory(trajectory_index);
            }
            ImGui::EndTable();
        }
    }

    void TrajectoriesInspector::render_trajectory(size_t trajectory_index)
    {
        auto &trajectory = m_trajectories[trajectory_index];
        sfg_imgui_vendor::PushIdGuard id_guard(&trajectory);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        auto expanded = ImGui::TreeNodeEx(("Trajectory " + std::to_string(trajectory_index)).c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_DrawLinesFull);
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        ImGui::Text("start:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::DragFloat("##time_from_start", &trajectory.m_delay_from_start, 0.1f, 0.0f, FLT_MAX);
        ImGui::TableNextColumn();

        if (ImGui::Button(s_add_button_text))
        {
            trajectory.m_points.emplace_back();
        }
        ImGui::SameLine();

        if (ImGui::Button(s_remove_button_text))
        {
        }

        if (!expanded)
        {
            return;
        }

        for (size_t point_index = 0; point_index < trajectory.m_points.size(); point_index++)
        {
            render_trajectory_point(trajectory_index, point_index);
        }
        ImGui::TreePop();
    }

    void TrajectoriesInspector::render_trajectory_point(size_t trajectory_index, size_t point_index)
    {
        auto &point = m_trajectories[trajectory_index].m_points[point_index];
        sfg_imgui_vendor::PushIdGuard id_guard(&point);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();

        if (ImGui::TreeNodeEx(("Waypoint " + std::to_string(point_index)).c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Leaf))
        {
            ImGui::TableNextColumn();
            ImGui::Text("last: ");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::DragFloat("##time_from_last", &point.m_delay_from_last, 0.1f, 0.0f, FLT_MAX);
            ImGui::TreePop();
        }

        ImGui::TableNextColumn();
    }
}