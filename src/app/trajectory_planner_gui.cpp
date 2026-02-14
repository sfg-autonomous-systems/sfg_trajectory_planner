#include "sfg_trajectory_planner/app/trajectory_planner_gui.hpp"

#include "sfg_trajectory_planner/app/core/grid.hpp"
#include "sfg_trajectory_planner/app/core/trajectory.hpp"

namespace sfg_trajectory_planner::app
{
    static constexpr auto s_main_window_flags = ImGuiWindowFlags_NoResize |
                                                ImGuiWindowFlags_NoBackground |
                                                ImGuiWindowFlags_NoSavedSettings |
                                                ImGuiWindowFlags_NoScrollbar |
                                                ImGuiWindowFlags_NoScrollWithMouse |
                                                ImGuiWindowFlags_NoCollapse |
                                                ImGuiWindowFlags_MenuBar;

    TrajectoryPlannerGui::TrajectoryPlannerGui(rclcpp::Node *node)
        : GuiElement(),
          m_scene(m_factory),
          m_renderer(m_camera, {0.0f, 0.0f, 0.0f}),
          m_selection_context(m_scene),
          m_scene_hierarchy(m_scene, m_selection_context),
          m_viewport(node, m_scene, m_camera, m_renderer, m_selection_context)
    {
        // Add supported scene object types to the factory.
        m_factory.register_object_type<app::core::Trajectory>(
            [&](engine::core::SceneObject::ConstructionKey key, engine::core::Scene &scene, uuids::uuid uuid)
            {
                return std::make_unique<app::core::Trajectory>(key, scene, uuid);
            },
            "Trajectory");
        m_factory.register_object_type<app::core::Grid>(
            [&](engine::core::SceneObject::ConstructionKey key, engine::core::Scene &scene, uuids::uuid uuid)
            {
                return std::make_unique<app::core::Grid>(key, scene, uuid);
            },
            "Grid");

        m_scene.create_object<app::core::Grid>("Grid");
    }

    void TrajectoryPlannerGui::render_internal()
    {
        auto &io = ImGui::GetIO();

        ImGuizmo::BeginFrame();
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("##main_window", nullptr, s_main_window_flags);

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Open"))
                {
                }

                if (ImGui::MenuItem("Save"))
                {
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

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
            // ToDo: Render inspector here.
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
        ImGui::GetWindowDrawList()->AddText(ImVec2(position.x + ImGui::GetStyle().FramePadding.x, position.y - 0.5f * text_size.y), ImGui::GetColorU32(ImGuiCol_Text), title.data());
    }
}