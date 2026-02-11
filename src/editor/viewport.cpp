#include "sfg_trajectory_planner/editor/viewport.hpp"

#include "sfg_trajectory_planner/core/gfx/utils.hpp"
#include "sfg_utils/ros_utils.hpp"

#include <magic_enum.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace sfg_trajectory_planner::editor
{
    Viewport::Viewport(rclcpp::Node *node, core::Scene &scene, SelectionContext &selection_context, core::gfx::Renderer &renderer)
        : GuiElement(),
          m_scene(scene),
          m_selection_context(selection_context),
          m_renderer(renderer)
    {
        // Declare and retrieve ROS parameters.
        m_camera.m_orbit_speed = sfg_utils::ros_utils::declare_parameter_if_not_declared(*node, "gui.viewport.camera.orbit_speed", m_camera.m_orbit_speed);
        m_camera.m_zoom_speed = sfg_utils::ros_utils::declare_parameter_if_not_declared(*node, "gui.viewport.camera.zoom_speed", m_camera.m_zoom_speed);
    }

    void Viewport::render_internal()
    {
        process_input();

        m_camera.m_viewport = {ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y, ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y};
        ImGuizmo::SetRect(m_camera.m_viewport.x, m_camera.m_viewport.y, m_camera.m_viewport.z, m_camera.m_viewport.w);
        ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
        update_camera_matrices();
        m_renderer.set_matrices(m_camera.m_view_matrix, m_camera.m_projection_matrix, m_camera.m_viewport);

        for (auto &object : m_scene.get_root()->get_children())
        {
            render_object(*object);
        }

        auto image = m_renderer.render();
        ImGui::GetBackgroundDrawList()->AddImage(
            static_cast<ImTextureID>(image),
            ImVec2(m_camera.m_viewport.x, m_camera.m_viewport.y),
            ImVec2(m_camera.m_viewport.x + m_camera.m_viewport.z, m_camera.m_viewport.y + m_camera.m_viewport.w),
            ImVec2(0.0f, 1.0f),
            ImVec2(1.0f, 0.0f));

        // ImGuizmo::ViewManipulate enables rotating the view by holding down the left mouse button and then moving the mouse.
        // But in our application we use the right mouse button for orbiting the camera, so we temporarily disable the left mouse button input.
        ImGuiIO &io = ImGui::GetIO();
        auto old_mouse_delta = io.MouseDelta;
        io.MouseDelta = ImVec2(0.0f, 0.0f);
        ImGuizmo::ViewManipulate(
            glm::value_ptr(m_camera.m_view_matrix),
            Config::s_view_gizmo_distance,
            ImVec2(m_camera.m_viewport.x + m_camera.m_viewport.z - Config::s_view_gizmo_size, m_camera.m_viewport.y),
            ImVec2(Config::s_view_gizmo_size, Config::s_view_gizmo_size),
            0);
        io.MouseDelta = old_mouse_delta;

        if (auto selected_object = m_selection_context.get_selected())
        {
            auto object_to_world_matrix = selected_object->get_object_to_world_matrix();

            if (m_renderer.add_gizmo(object_to_world_matrix, m_selection_context.get_gizmo_operation(), m_selection_context.get_gizmo_mode()))
            {
                selected_object->get_transform().set_matrix(selected_object->get_parent()->get_world_to_object_matrix() * object_to_world_matrix);
            }
        }
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
                m_camera.m_focus_point = glm::vec3(selected_object->get_object_to_world_matrix()[3]);
            }
        }

        // Implement orbiting. If the right mouse button is held, adjust the camera orientation based on mouse movement.
        if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
        {
            m_camera.m_orientation.y = glm::clamp(m_camera.m_orientation.y - io.MouseDelta.y * m_camera.m_orbit_speed, m_camera.s_min_pitch, m_camera.s_max_pitch);
            m_camera.m_orientation.z += -io.MouseDelta.x * m_camera.m_orbit_speed;
        }

        // Implement zooming. Use the mouse wheel to adjust the camera distance.
        m_camera.m_zoom_level = glm::clamp(m_camera.m_zoom_level - ImGui::GetIO().MouseWheel * m_camera.m_zoom_speed, m_camera.s_near_plane, m_camera.s_far_plane);

        // Implement panning. If the middle mouse button is clicked, record the starting point for panning.
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle))
        {
            // The starting point for panning is the intersection between the mouse ray and the plane defined by the camera's focus point and inverse of the camera's forward vector.
            auto ray = core::gfx::utils::screen_space_to_ray(glm::vec2(ImGui::GetMousePos().x, ImGui::GetMousePos().y), m_camera.m_view_matrix, m_camera.m_projection_matrix, m_camera.m_viewport);
            auto distance = core::gfx::utils::intersect_ray_plane(ray, m_camera.m_focus_point, glm::inverse(m_camera.m_view_matrix) * core::gfx::utils::s_forward);
            // Because the plane is oriented towards the camera, we can always assume that we hit the plane.
            auto intersection = ray.origin + distance * ray.direction;
            m_camera.m_pan_start = intersection;
        }
        else if (ImGui::IsMouseDown(ImGuiMouseButton_Middle))
        {
            // If the middle mouse button is held, calculate the pan offset and adjust the camera focus point.
            auto ray = core::gfx::utils::screen_space_to_ray(glm::vec2(ImGui::GetMousePos().x, ImGui::GetMousePos().y), m_camera.m_view_matrix, m_camera.m_projection_matrix, m_camera.m_viewport);
            auto distance = core::gfx::utils::intersect_ray_plane(ray, m_camera.m_focus_point, glm::inverse(m_camera.m_view_matrix) * core::gfx::utils::s_forward);
            auto intersection = ray.origin + distance * ray.direction;
            m_camera.m_focus_point -= intersection - m_camera.m_pan_start;
        }
    }

    void Viewport::render_object(core::SceneObject &object)
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
        ImGui::SetNextWindowPos(ImVec2(m_camera.m_viewport.x, m_camera.m_viewport.y));

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

            index = magic_enum::enum_index(m_camera.m_projection).value();

            if (ImGui::Combo("Camera Projection", &index, "Orthographic\0Perspective\0"))
            {
                m_camera.m_projection = magic_enum::enum_value<Camera::Projection>(index);

                switch (m_camera.m_projection)
                {
                // We switched from perspective to orthographic mode.
                case Camera::Projection::Orthographic:
                    m_camera.m_zoom_level = glm::tan(Camera::s_vertical_fov / 2.0f) * m_camera.m_zoom_level;
                    ImGuizmo::SetOrthographic(true);
                    break;

                // We switched from orthographic to perspective mode.
                case Camera::Projection::Perspective:
                    m_camera.m_zoom_level = m_camera.m_zoom_level / glm::tan(Camera::s_vertical_fov / 2.0f);
                    ImGuizmo::SetOrthographic(false);
                    break;
                default:
                    break;
                }
            }
        }
        ImGui::EndChild();
    }

    void Viewport::update_camera_matrices()
    {
        auto viewport_size = ImGui::GetContentRegionAvail();
        auto aspect_ratio = viewport_size.x / viewport_size.y;

        switch (m_camera.m_projection)
        {
        case Camera::Projection::Orthographic:
            m_camera.m_projection_matrix = glm::ortho(
                -aspect_ratio * m_camera.m_zoom_level,
                aspect_ratio * m_camera.m_zoom_level,
                -m_camera.m_zoom_level,
                m_camera.m_zoom_level,
                m_camera.s_near_plane,
                m_camera.s_far_plane);
            break;
        case Camera::Projection::Perspective:
            m_camera.m_projection_matrix = glm::perspective(m_camera.s_vertical_fov, aspect_ratio, m_camera.s_near_plane, m_camera.s_far_plane);
            break;
        default:
            break;
        }

        if (ImGuizmo::IsUsingViewManipulate())
        {
            glm::mat4 view_inverse = glm::inverse(m_camera.m_view_matrix);
            glm::vec3 camera_forward = glm::normalize(view_inverse * core::gfx::utils::s_forward);
            m_camera.m_orientation.y = -glm::asin(glm::clamp(camera_forward.y, -1.0f, 1.0f));

            const auto epsilon = 0.001f;

            if ((camera_forward.x * camera_forward.x + camera_forward.z * camera_forward.z) > epsilon)
            {
                m_camera.m_orientation.z = glm::atan(camera_forward.x, camera_forward.z);
            }
        }

        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), m_camera.m_orientation.z, core::gfx::utils::s_up.xyz()) * glm::rotate(glm::mat4(1.0f), m_camera.m_orientation.y, core::gfx::utils::s_right.xyz());
        glm::mat4 translation = glm::translate(glm::mat4(1.0f), m_camera.m_focus_point);
        // If we are in perspective mode, we need to translate the camera back by the zoom level to maintain the correct distance from the focus point.
        // Otherwise, in orthographic mode, we translate the camera back by half the far plane distance to ensure the entire scene is visible.
        glm::mat4 center_offset = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, m_camera.m_projection == Camera::Projection::Perspective ? m_camera.m_zoom_level : Camera::s_far_plane / 2.0f));
        m_camera.m_view_matrix = glm::inverse(translation * rotation * center_offset);
    }
}