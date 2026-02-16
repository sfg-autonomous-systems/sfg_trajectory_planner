#include "sfg_trajectory_planner/engine/editor/viewport.hpp"

#include <magic_enum.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "sfg_trajectory_planner/engine/core/gfx/renderer.hpp"
#include "sfg_trajectory_planner/engine/core/gfx/utils.hpp"
#include "sfg_utils/ros_utils.hpp"

namespace sfg_trajectory_planner::engine::editor
{
    Viewport::Viewport(rclcpp::Node *node, engine::core::Scene &scene, engine::core::gfx::Camera &camera, engine::core::gfx::Renderer &renderer, SelectionContext &selection_context)
        : GuiElement(),
          m_scene(scene),
          m_camera(camera),
          m_renderer(renderer),
          m_selection_context(selection_context)
    {
        // Declare and retrieve ROS parameters.
        m_config.m_orbit_speed = sfg_utils::ros_utils::declare_parameter_if_not_declared(*node, "gui.viewport.camera.orbit_speed", m_config.m_orbit_speed);
        m_config.m_zoom_speed = sfg_utils::ros_utils::declare_parameter_if_not_declared(*node, "gui.viewport.camera.zoom_speed", m_config.m_zoom_speed);
    }

    void Viewport::render_internal()
    {
        process_input();

        // Set viewport for camera and ImGuizmo.
        m_camera.set_viewport({ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y, ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y});
        ImGuizmo::SetRect(m_camera.get_viewport().x, m_camera.get_viewport().y, m_camera.get_viewport().z, m_camera.get_viewport().w);

        // Finalize camera matrices after processing input and before rendering.
        m_camera.update();

        // Prepare rendering.
        ImGuizmo::SetOrthographic(m_camera.get_projection() == engine::core::gfx::Camera::Projection::Orthographic);
        ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
        m_renderer.set_matrices(m_camera.get_view_matrix(), m_camera.get_projection_matrix(), m_camera.get_viewport());

        for (auto &object : m_scene.get_root()->get_children())
        {
            render_object(*object);
        }

        if (auto editor = m_selection_context.get_selected_editor())
        {
            editor->render_editor(m_renderer);
        }

        // Display the rendered image as an ImGui background.
        auto image = m_renderer.render();
        ImGui::GetBackgroundDrawList()->AddImage(
            static_cast<ImTextureID>(image),
            ImVec2(m_camera.get_viewport().x, m_camera.get_viewport().y),
            ImVec2(m_camera.get_viewport().x + m_camera.get_viewport().z, m_camera.get_viewport().y + m_camera.get_viewport().w),
            ImVec2(0.0f, 1.0f),
            ImVec2(1.0f, 0.0f));

        // Display the view manipulation gizmo in the top-right corner of the viewport. Note that ImGuizmo::ViewManipulate enables
        // rotating the view by holding down the left mouse button and then moving the mouse. But in our application we use the
        // right mouse button for orbiting the camera, so we temporarily disable the left mouse button input.
        ImGuiIO &io = ImGui::GetIO();
        auto old_mouse_delta = io.MouseDelta;
        io.MouseDelta = ImVec2(0.0f, 0.0f);

        auto view_matrix = m_camera.get_view_matrix();
        ImGuizmo::ViewManipulate(
            glm::value_ptr(view_matrix),
            Config::s_view_gizmo_distance,
            ImVec2(m_camera.get_viewport().x + m_camera.get_viewport().z - Config::s_view_gizmo_size, m_camera.get_viewport().y),
            ImVec2(Config::s_view_gizmo_size, Config::s_view_gizmo_size),
            0);

        io.MouseDelta = old_mouse_delta;

        // If the user is currently manipulating the view, synchronize the camera's orientation and focus point based on the modified view matrix.
        if (ImGuizmo::IsUsingViewManipulate())
        {
            m_camera.synchronize_from_matrix(view_matrix);
        }

        // Render viewport settings GUI.
        render_settings();
    }

    void Viewport::process_input()
    {
        auto &io = ImGui::GetIO();

        if (!ImGui::IsWindowHovered() || ImGuizmo::IsUsingViewManipulate() || ImGuizmo::IsUsing())
        {
            return;
        }

        if (!io.WantCaptureKeyboard && ImGui::IsKeyPressed(ImGuiKey_W))
        {
            m_selection_context.set_gizmo_operation(ImGuizmo::TRANSLATE);
        }

        if (!io.WantCaptureKeyboard && ImGui::IsKeyPressed(ImGuiKey_E))
        {
            m_selection_context.set_gizmo_operation(ImGuizmo::ROTATE);
        }

        if (!io.WantCaptureKeyboard && ImGui::IsKeyPressed(ImGuiKey_R))
        {
            m_selection_context.set_gizmo_operation(ImGuizmo::SCALE);
        }

        if (!io.WantCaptureKeyboard && ImGui::IsKeyPressed(ImGuiKey_F))
        {
            if (auto selected_object = m_selection_context.get_selected())
            {
                m_camera.focus_on(glm::vec3(selected_object->get_object_to_world_matrix()[3]));
            }
        }

        // Implement orbiting. If the right mouse button is held, adjust the camera orientation based on mouse movement.
        if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
        {
            m_camera.orbit(io.MouseDelta.x * m_config.m_orbit_speed, io.MouseDelta.y * m_config.m_orbit_speed);
        }

        // Implement zooming. Use the mouse wheel to adjust the camera distance.
        m_camera.zoom(ImGui::GetIO().MouseWheel * m_config.m_zoom_speed);

        // Implement panning. If the middle mouse button is clicked, record the starting point for panning.
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle))
        {
            m_camera.start_pan(glm::vec2(ImGui::GetMousePos().x, ImGui::GetMousePos().y));
        }
        // If the middle mouse button is held, adjust the camera position based on mouse movement.
        else if (ImGui::IsMouseDown(ImGuiMouseButton_Middle))
        {
            m_camera.pan(glm::vec2(ImGui::GetMousePos().x, ImGui::GetMousePos().y));
        }
    }

    void Viewport::render_object(engine::core::SceneObject &object)
    {
        if (!object.is_visible())
        {
            return;
        }

        object.render_object(m_renderer);

        for (auto &child : object.get_children())
        {
            render_object(*child);
        }
    }

    void Viewport::render_settings()
    {
        ImGui::SetNextWindowPos(ImVec2(m_camera.get_viewport().x, m_camera.get_viewport().y));

        if (ImGui::BeginChild("Controls", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY))
        {
            const ImGuizmo::OPERATION supported_gizmo_operations[] = {ImGuizmo::TRANSLATE, ImGuizmo::ROTATE, ImGuizmo::SCALE};
            const char *supported_gizmo_operation_displaynames = "Translate\0Rotate\0Scale\0";
            std::int32_t index;

            for (size_t i = 0; i < std::size(supported_gizmo_operations); ++i)
            {
                if (m_selection_context.get_gizmo_operation() == supported_gizmo_operations[i])
                {
                    index = i;
                    break;
                }
            }

            if (ImGui::Combo("Gizmo Operation", &index, supported_gizmo_operation_displaynames))
            {
                m_selection_context.set_gizmo_operation(supported_gizmo_operations[index]);
            }

            index = magic_enum::enum_index(m_selection_context.get_gizmo_mode()).value();

            if (ImGui::Combo("Gizmo Mode", &index, "Local\0World\0"))
            {
                m_selection_context.set_gizmo_mode(magic_enum::enum_value<ImGuizmo::MODE>(index));
            }

            index = magic_enum::enum_index(m_camera.get_projection()).value();

            if (ImGui::Combo("Camera Projection", &index, "Perspective\0Orthographice\0"))
            {
                m_camera.set_projection(magic_enum::enum_value<engine::core::gfx::Camera::Projection>(index));
            }
        }
        ImGui::EndChild();
    }
}