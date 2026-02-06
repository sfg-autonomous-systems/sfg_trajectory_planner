#include "sfg_trajectory_planner/trajectories_inspector.hpp"

namespace sfg_trajectory_planner
{
    void TrajectoriesInspector::render_internal()
    {
        if (ImGui::CollapsingHeader("Trajectories", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("IsItemHovered: %d", ImGui::IsItemHovered());
            for (int i = 0; i < 5; i++)
                ImGui::Text("Some content %d", i);
        }
    }
}