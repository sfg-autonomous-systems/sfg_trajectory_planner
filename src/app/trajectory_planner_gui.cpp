#include "sfg_trajectory_planner/app/trajectory_planner_gui.hpp"

#include "sfg_trajectory_planner/app/core/grid.hpp"
#include "sfg_trajectory_planner/app/core/trajectory.hpp"
#include "sfg_trajectory_planner/app/editor/grid_editor.hpp"
#include "sfg_trajectory_planner/app/editor/trajectory_editor.hpp"

namespace sfg_trajectory_planner::app
{
    static constexpr auto s_main_window_flags = ImGuiWindowFlags_NoResize |
                                                ImGuiWindowFlags_NoBackground |
                                                ImGuiWindowFlags_NoSavedSettings |
                                                ImGuiWindowFlags_NoScrollbar |
                                                ImGuiWindowFlags_NoScrollWithMouse |
                                                ImGuiWindowFlags_NoCollapse |
                                                ImGuiWindowFlags_MenuBar;

    static constexpr SDL_DialogFileFilter s_file_dialog_filters[] = {"Scene Files", "yaml;yml"};

    std::mutex TrajectoryPlannerGui::s_file_dialog_mutex;
    TrajectoryPlannerGui::FileDialogResult TrajectoryPlannerGui::s_file_dialog_result;

    TrajectoryPlannerGui::TrajectoryPlannerGui(rclcpp::Node *node)
        : GuiElement(),
          m_scene(m_scene_object_factory),
          m_renderer(m_camera, {0.0f, 0.0f, 0.0f}),
          m_selection_context(m_scene, m_scene_object_editor_factory),
          m_scene_hierarchy(m_scene, m_selection_context),
          m_viewport(node, m_scene, m_camera, m_renderer, m_selection_context),
          m_inspector(m_selection_context)
    {
        // Add supported scene object types to the factory.
        m_scene_object_factory.register_type<engine::core::SceneObject, engine::core::SceneObject>("Scene Object");
        m_scene_object_factory.register_type<app::core::Trajectory, app::core::Trajectory>("Trajectory");
        m_scene_object_factory.register_type<app::core::Grid, app::core::Grid>("Grid");

        // Do the same for scene object editors.
        m_scene_object_editor_factory.register_type<engine::core::SceneObject, engine::editor::SceneObjectEditor>();
        m_scene_object_editor_factory.register_type<app::core::Trajectory, app::editor::TrajectoryEditor>();
        m_scene_object_editor_factory.register_type<app::core::Grid, app::editor::GridEditor>();

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
                std::lock_guard lock(s_file_dialog_mutex);
                auto enabled = s_file_dialog_result.m_state == FileDialogResult::State::Idle;

                if (ImGui::MenuItem("Open", nullptr, false, enabled))
                {
                    SDL_ShowOpenFileDialog(
                        file_dialog_callback,
                        reinterpret_cast<void *>(static_cast<std::uintptr_t>(FileDialogResult::Mode::Open)),
                        SDL_GL_GetCurrentWindow(),
                        s_file_dialog_filters, sizeof(s_file_dialog_filters) / sizeof(s_file_dialog_filters[0]),
                        nullptr,
                        false);
                    s_file_dialog_result.m_state = FileDialogResult::State::WaitingForUserInput;
                }

                if (ImGui::MenuItem("Save", nullptr, false, enabled))
                {
                    SDL_ShowSaveFileDialog(
                        file_dialog_callback,
                        reinterpret_cast<void *>(static_cast<std::uintptr_t>(FileDialogResult::Mode::Save)),
                        SDL_GL_GetCurrentWindow(),
                        s_file_dialog_filters, sizeof(s_file_dialog_filters) / sizeof(s_file_dialog_filters[0]),
                        nullptr);
                    s_file_dialog_result.m_state = FileDialogResult::State::WaitingForUserInput;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        {
            std::lock_guard lock(s_file_dialog_mutex);

            if (s_file_dialog_result.m_state == FileDialogResult::State::WaitingForGuiProcessing && s_file_dialog_result.m_path)
            {
                if (s_file_dialog_result.m_path)
                {
                    // ToDo: Process the file path (open or save the scene depending on the mode).
                }
                s_file_dialog_result.m_path = std::nullopt;
                s_file_dialog_result.m_state = FileDialogResult::State::Idle;
            }
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
            m_inspector.render();
            ImGui::EndChild();
            auto inspector_rect_min = ImGui::GetItemRectMin();
            render_title(inspector_displayname, ImVec2(inspector_rect_min.x, inspector_rect_min.y));

            ImGui::EndTable();
        }
        ImGui::End();
    }

    void TrajectoryPlannerGui::file_dialog_callback(void *user_data, const char *const *file_list, int)
    {
        std::lock_guard lock(s_file_dialog_mutex);

        if (file_list && file_list[0])
        {
            s_file_dialog_result.m_path = file_list[0];
        }
        s_file_dialog_result.m_mode = static_cast<FileDialogResult::Mode>(reinterpret_cast<std::uintptr_t>(user_data));
        s_file_dialog_result.m_state = FileDialogResult::State::WaitingForGuiProcessing;
    }

    void TrajectoryPlannerGui::render_title(std::string_view title, const ImVec2 &position)
    {
        auto text_size = ImGui::CalcTextSize(title.data());
        ImGui::GetWindowDrawList()->AddText(ImVec2(position.x + ImGui::GetStyle().FramePadding.x, position.y - 0.5f * text_size.y), ImGui::GetColorU32(ImGuiCol_Text), title.data());
    }
}