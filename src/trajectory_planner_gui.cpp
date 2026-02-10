#include "sfg_trajectory_planner/trajectory_planner_gui.hpp"

#include "sfg_trajectory_planner/gfx_utils.hpp"

namespace sfg_trajectory_planner
{
    TrajectoryPlannerGui::TrajectoryPlannerGui(rclcpp::Node *node)
        : GuiElement(),
          m_scene_hierarchy(m_scene),
          m_viewport(node, m_scene)
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
        if (ImGui::BeginTable("TopColumns", 3, ImGuiTableFlags_Resizable))
        {
            constexpr auto scene_hierarchy_displayname = "Scene Hierarchy";
            constexpr auto viewport_displayname = "Viewport";
            constexpr auto inspector_displayname = "Inspector";

            ImGui::TableSetupColumn(scene_hierarchy_displayname, ImGuiTableColumnFlags_WidthStretch, 1.0f);
            ImGui::TableSetupColumn(viewport_displayname, ImGuiTableColumnFlags_WidthStretch, 3.0f);
            ImGui::TableSetupColumn(inspector_displayname, ImGuiTableColumnFlags_WidthStretch, 1.0f);

            ImGui::TableNextColumn();
            ImGui::BeginChild(scene_hierarchy_displayname, ImVec2(0, 0), ImGuiChildFlags_Border);
            m_scene_hierarchy.render();
            ImGui::EndChild();
            auto scene_hierarchy_rect_min = ImGui::GetItemRectMin();
            render_title(scene_hierarchy_displayname, ImVec2(scene_hierarchy_rect_min.x, scene_hierarchy_rect_min.y));

            ImGui::TableNextColumn();
            ImGui::BeginChild(viewport_displayname, ImVec2(0, 0), ImGuiChildFlags_Border);
            m_viewport.render();
            ImGui::EndChild();
            auto viewport_rect_min = ImGui::GetItemRectMin();
            render_title(viewport_displayname, ImVec2(viewport_rect_min.x, viewport_rect_min.y));

            ImGui::TableNextColumn();
            ImGui::BeginChild(inspector_displayname, ImVec2(0.0f, 0.0f), ImGuiChildFlags_Border);
            m_inspector.render();
            ImGui::EndChild();
            auto inspector_rect_min = ImGui::GetItemRectMin();
            render_title(inspector_displayname, ImVec2(inspector_rect_min.x, inspector_rect_min.y));

            ImGui::EndTable();
        }
        ImGui::End();
    }

    void TrajectoryPlannerGui::render_title(std::string_view title, const ImVec2 &position)
    {
        auto text_size = ImGui::CalcTextSize(title.data());
        ImGui::GetForegroundDrawList()->AddText(ImVec2(position.x + ImGui::GetStyle().FramePadding.x, position.y - 0.5f * text_size.y), ImGui::GetColorU32(ImGuiCol_Text), title.data());
    }
}