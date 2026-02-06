#include "sfg_trajectory_planner/trajectory_planner_gui.hpp"

#include "sfg_trajectory_planner/gfx_math.hpp"

namespace sfg_trajectory_planner
{
    TrajectoryPlannerGui::TrajectoryPlannerGui(rclcpp::Node *node) : GuiElement(), m_viewport(node)
    {
    }

    void TrajectoryPlannerGui::render_internal()
    {
        auto &io = ImGui::GetIO();

        ImGuizmo::BeginFrame();
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin(
            "##Main Window",
            nullptr,
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse);
        if (ImGui::BeginTable("TopColumns", 2, ImGuiTableFlags_Resizable))
        {
            ImGui::TableSetupColumn("Viewport", ImGuiTableColumnFlags_WidthStretch, 2.0f);
            ImGui::TableSetupColumn("Inspector", ImGuiTableColumnFlags_WidthStretch, 1.0f);
            ImGui::TableNextColumn();
            render_viewport();
            ImGui::TableNextColumn();
            render_trajectories_inspector();
            render_transform_inspector();
            ImGui::EndTable();
        }
        ImGui::End();
    }

    void TrajectoryPlannerGui::render_viewport()
    {
        m_viewport.render();
    }

    void TrajectoryPlannerGui::render_trajectories_inspector()
    {
        ImGui::BeginChild("Trajectories Inspector", ImVec2(0, ImGui::GetContentRegionAvail().y * 0.5f), ImGuiChildFlags_Border);

        if (ImGui::TreeNodeEx("Trajectories"))
        {
            for (size_t i = 0; i < m_trajectories.size(); ++i)
            {
                auto &trajectory = m_trajectories[i];

                if (ImGui::TreeNodeEx((std::string("Trajectory ") + std::to_string(i)).data()))
                {
                    ImGui::Text("Number of Points: %zu", trajectory.points.size());
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }

        if (ImGui::Button("New"))
        {
            m_trajectories.push_back(Trajectory{});
        }

        ImGui::SameLine();

        if (ImGui::Button("Delete"))
        {
            m_trajectories.clear();
        }
        ImGui::EndChild();
    }

    void TrajectoryPlannerGui::render_transform_inspector()
    {
        ImGui::BeginChild("Transform Inspector", ImVec2(0, 0), ImGuiChildFlags_Border);

        float position[3] = {0.0f, 0.0f, 0.0f};
        float rotation[3] = {0.0f, 0.0f, 0.0f};
        float scale[3] = {1.0f, 1.0f, 1.0f};

        ImGui::InputFloat3("Position", position);
        ImGui::InputFloat3("Rotation", rotation);
        ImGui::InputFloat3("Scale", scale);
        ImGui::EndChild();
    }
}