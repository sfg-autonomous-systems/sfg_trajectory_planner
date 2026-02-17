#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <imgui/imgui.h>
#include <imgui/ImGuizmo.h>
#include <rclcpp/rclcpp.hpp>
#include <SDL3/SDL.h>

#include "sfg_imgui_vendor/gui_element.hpp"
#include "sfg_trajectory_planner/engine/core/gfx/camera.hpp"
#include "sfg_trajectory_planner/engine/core/gfx/renderer.hpp"
#include "sfg_trajectory_planner/engine/core/scene.hpp"
#include "sfg_trajectory_planner/engine/core/scene_object_factory.hpp"
#include "sfg_trajectory_planner/engine/editor/inspector.hpp"
#include "sfg_trajectory_planner/engine/editor/scene_object_editor_factory.hpp"
#include "sfg_trajectory_planner/engine/editor/scene_hierarchy.hpp"
#include "sfg_trajectory_planner/engine/editor/selection_context.hpp"
#include "sfg_trajectory_planner/engine/editor/viewport.hpp"

namespace sfg_trajectory_planner::app
{
    class TrajectoryPlannerGui : public sfg_imgui_vendor::GuiElement
    {
    public:
        TrajectoryPlannerGui(rclcpp::Node *node);

    protected:
        void render_internal() override;

    private:
        struct FileDialogResult
        {
            enum class Mode
            {
                None,
                Open,
                Save
            };

            enum class State
            {
                Idle,
                WaitingForUserInput,
                WaitingForGuiProcessing

            };

            std::optional<std::string> m_path;
            Mode m_mode;
            State m_state = State::Idle;
        };

        static void SDLCALL file_dialog_callback(void *user_data, const char *const *file_list, int filter);

        void render_title(std::string_view title, const ImVec2 &position);
        void handle_file_dialog_result();

        // File handling.
        static std::mutex s_file_dialog_mutex;
        static FileDialogResult s_file_dialog_result;

        // Factories for creating scene objects and their corresponding editors.
        engine::core::SceneObjectFactory m_scene_object_factory;
        engine::editor::SceneObjectEditorFactory m_scene_object_editor_factory;

        // Core engine components.
        engine::core::Scene m_scene;
        engine::core::gfx::Camera m_camera;
        engine::core::gfx::Renderer m_renderer;

        engine::editor::SelectionContext m_selection_context;

        // GUI elements for the different windows.
        engine::editor::SceneHierarchy m_scene_hierarchy;
        engine::editor::Viewport m_viewport;
        engine::editor::Inspector m_inspector;
    };
}