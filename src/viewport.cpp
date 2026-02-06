#include "sfg_trajectory_planner/viewport.hpp"

#include "sfg_trajectory_planner/gfx_math.hpp"
#include "sfg_utils/ros_utils.hpp"

#include <magic_enum.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace sfg_trajectory_planner
{
    Viewport::Viewport(rclcpp::Node *node) : GuiElement()
    {
        // Declare and retrieve ROS parameters.
        m_config.m_grid_origin = glm::vec3(
            sfg_utils::ros_utils::declare_parameter_if_not_declared(*node, "gui.viewport.grid.origin.x", m_config.m_grid_origin.x),
            sfg_utils::ros_utils::declare_parameter_if_not_declared(*node, "gui.viewport.grid.origin.y", m_config.m_grid_origin.y),
            sfg_utils::ros_utils::declare_parameter_if_not_declared(*node, "gui.viewport.grid.origin.z", m_config.m_grid_origin.z));
        m_config.m_grid_scale = glm::vec3(
            sfg_utils::ros_utils::declare_parameter_if_not_declared(*node, "gui.viewport.grid.scale.x", m_config.m_grid_scale.x),
            sfg_utils::ros_utils::declare_parameter_if_not_declared(*node, "gui.viewport.grid.scale.y", m_config.m_grid_scale.y),
            sfg_utils::ros_utils::declare_parameter_if_not_declared(*node, "gui.viewport.grid.scale.z", m_config.m_grid_scale.z));
        m_config.m_grid_size = sfg_utils::ros_utils::declare_parameter_if_not_declared(*node, "gui.viewport.grid.size", m_config.m_grid_size);
        m_camera.m_orbit_speed = sfg_utils::ros_utils::declare_parameter_if_not_declared(*node, "gui.viewport.camera.orbit_speed", m_camera.m_orbit_speed);
        m_camera.m_zoom_speed = sfg_utils::ros_utils::declare_parameter_if_not_declared(*node, "gui.viewport.camera.zoom_speed", m_camera.m_zoom_speed);

        m_grid_matrix = glm::translate(glm::mat4(1.0f), m_config.m_grid_origin);
        m_grid_matrix = glm::scale(m_grid_matrix, m_config.m_grid_scale);
    }

    void Viewport::render_internal()
    {
        process_input();

        m_camera.m_viewport = {ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y, ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y};
        ImGuizmo::SetRect(m_camera.m_viewport.x, m_camera.m_viewport.y, m_camera.m_viewport.z, m_camera.m_viewport.w);
        ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
        update_camera_matrices();
        ImGuizmo::DrawGrid(glm::value_ptr(m_camera.m_view_matrix), glm::value_ptr(m_camera.m_projection_matrix), glm::value_ptr(m_grid_matrix), m_config.m_grid_size);
        // ImGuizmo::ViewManipulate enables rotating the view by holding down the left mouse button and then moving the mouse.
        // But in our application we use the right mouse button for orbiting the camera, so we temporarily disable the left mouse button input.
        ImGuiIO &io = ImGui::GetIO();
        auto old_mouse_delta = io.MouseDelta;
        io.MouseDelta = ImVec2(0, 0);
        ImGuizmo::ViewManipulate(
            glm::value_ptr(m_camera.m_view_matrix),
            Config::c_view_gizmo_distance,
            ImVec2(m_camera.m_viewport.z - Config::c_view_gizmo_size, m_camera.m_viewport.y),
            ImVec2(Config::c_view_gizmo_size, Config::c_view_gizmo_size),
            0);
        io.MouseDelta = old_mouse_delta;

        render_settings();
    }

    void Viewport::process_input()
    {
        auto &io = ImGui::GetIO();

        if (ImGui::IsKeyPressed(ImGuiKey_W))
        {
            m_gizmo_operation = ImGuizmo::TRANSLATE;
        }

        if (ImGui::IsKeyPressed(ImGuiKey_E))
        {
            m_gizmo_operation = ImGuizmo::ROTATE;
        }

        if (ImGui::IsKeyPressed(ImGuiKey_R))
        {
            m_gizmo_operation = ImGuizmo::SCALE;
        }

        if (!ImGui::IsWindowHovered() || ImGuizmo::IsUsingViewManipulate() || ImGuizmo::IsUsing())
        {
            return;
        }

        // Implement orbiting. If the right mouse button is held, adjust the camera orientation based on mouse movement.
        if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
        {
            m_camera.m_orientation.y = glm::clamp(m_camera.m_orientation.y - io.MouseDelta.y * m_camera.m_orbit_speed, m_camera.c_min_pitch, m_camera.c_max_pitch);
            m_camera.m_orientation.z += -io.MouseDelta.x * m_camera.m_orbit_speed;
        }

        // Implement zooming. Use the mouse wheel to adjust the camera distance.
        m_camera.m_zoom_level = glm::clamp(m_camera.m_zoom_level - ImGui::GetIO().MouseWheel * m_camera.m_zoom_speed, m_camera.c_near_plane, m_camera.c_far_plane);

        // Implement panning. If the middle mouse button is clicked, record the starting point for panning.
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle))
        {
            // The starting point for panning is the intersection between the mouse ray and the plane defined by the camera's focus point and inverse of the camera's forward vector.
            auto ray = gfx_math::screen_space_to_ray(glm::vec2(ImGui::GetMousePos().x, ImGui::GetMousePos().y), m_camera.m_view_matrix, m_camera.m_projection_matrix, m_camera.m_viewport);
            auto distance = gfx_math::intersect_ray_plane(ray, m_camera.m_focus_point, glm::inverse(m_camera.m_view_matrix) * gfx_math::forward);
            // Because the plane is oriented towards the camera, we can always assume that we hit the plane.
            auto intersection = ray.origin + distance * ray.direction;
            m_camera.m_pan_start = intersection;
        }
        else if (ImGui::IsMouseDown(ImGuiMouseButton_Middle))
        {
            // If the middle mouse button is held, calculate the pan offset and adjust the camera focus point.
            auto ray = gfx_math::screen_space_to_ray(glm::vec2(ImGui::GetMousePos().x, ImGui::GetMousePos().y), m_camera.m_view_matrix, m_camera.m_projection_matrix, m_camera.m_viewport);
            auto distance = gfx_math::intersect_ray_plane(ray, m_camera.m_focus_point, glm::inverse(m_camera.m_view_matrix) * gfx_math::forward);
            auto intersection = ray.origin + distance * ray.direction;
            m_camera.m_focus_point -= intersection - m_camera.m_pan_start;
        }
    }

    void Viewport::render_settings()
    {
        ImGui::SetNextWindowPos(ImVec2(m_camera.m_viewport.x, m_camera.m_viewport.y));

        if (ImGui::BeginChild("Controls", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY))
        {
            const ImGuizmo::OPERATION supported_gizmo_operations[] = {ImGuizmo::TRANSLATE, ImGuizmo::ROTATE, ImGuizmo::SCALE};
            const char *supported_gizmo_operation_displaynames = "Translate\0Rotate\0Scale\0";

            ImGui::Text("Gizmo Operation:   ");
            ImGui::SameLine();
            std::int32_t index;

            for (size_t i = 0; i < std::size(supported_gizmo_operations); ++i)
            {
                if (m_gizmo_operation == supported_gizmo_operations[i])
                {
                    index = i;
                    break;
                }
            }

            if (ImGui::Combo("##guizmo_operation", &index, supported_gizmo_operation_displaynames))
            {
                m_gizmo_operation = supported_gizmo_operations[index];
            }

            ImGui::Text("Gizmo Mode:        ");
            ImGui::SameLine();
            index = magic_enum::enum_index(m_gizmo_mode).value();

            if (ImGui::Combo("##guizmo_mode", &index, "Local\0World\0"))
            {
                m_gizmo_mode = magic_enum::enum_value<ImGuizmo::MODE>(index);
            }

            ImGui::Text("Camera Projection: ");
            ImGui::SameLine();
            index = magic_enum::enum_index(m_camera.m_projection).value();

            if (ImGui::Combo("##camera_projection", &index, "Orthographic\0Perspective\0"))
            {
                m_camera.m_projection = magic_enum::enum_value<Camera::Projection>(index);

                switch (m_camera.m_projection)
                {
                // We switched from perspective to orthographic mode.
                case Camera::Projection::Orthographic:
                    m_camera.m_zoom_level = glm::tan(Camera::c_vertical_fov / 2.0f) * m_camera.m_zoom_level;
                    ImGuizmo::SetOrthographic(true);
                    break;

                // We switched from orthographic to perspective mode.
                case Camera::Projection::Perspective:
                    m_camera.m_zoom_level = m_camera.m_zoom_level / glm::tan(Camera::c_vertical_fov / 2.0f);
                    ImGuizmo::SetOrthographic(false);
                    break;
                default:
                    break;
                }
            }
        }
        ImGui::EndChild();
    }

    void Viewport::render_line(glm::vec3 start, glm::vec3 end, ImU32 color, float thickness)
    {
        auto project = [&](glm::vec3 world_space) -> std::optional<ImVec2>
        {
            // We need to flip the y coordinate because glm::project assumes the origin is at the bottom-left while ImGui assumes the origin is at the top-left.
            auto viewport = glm::vec4(m_camera.m_viewport.x, m_camera.m_viewport.y + m_camera.m_viewport.w, m_camera.m_viewport.z, -m_camera.m_viewport.w);
            glm::vec3 screen_space = glm::project(world_space, m_camera.m_view_matrix, m_camera.m_projection_matrix, viewport);

            if (screen_space.z < 0.0f)
            {
                return std::nullopt;
            }
            return ImVec2(screen_space.x, screen_space.y);
        };

        auto start_screen_space = project(start);
        auto end_screen_space = project(end);

        if (start_screen_space && end_screen_space)
        {
            ImGui::GetWindowDrawList()->AddLine(*start_screen_space, *end_screen_space, color, thickness);
        }
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
                m_camera.c_near_plane,
                m_camera.c_far_plane);
            break;
        case Camera::Projection::Perspective:
            m_camera.m_projection_matrix = glm::perspective(m_camera.c_vertical_fov, aspect_ratio, m_camera.c_near_plane, m_camera.c_far_plane);
            break;
        default:
            break;
        }

        if (ImGuizmo::IsUsingViewManipulate())
        {
            glm::mat4 view_inverse = glm::inverse(m_camera.m_view_matrix);
            glm::vec3 camera_forward = glm::normalize(view_inverse * gfx_math::forward);
            m_camera.m_orientation.y = -glm::asin(glm::clamp(camera_forward.y, -1.0f, 1.0f));

            const auto epsilon = 0.001f;

            if ((camera_forward.x * camera_forward.x + camera_forward.z * camera_forward.z) > epsilon)
            {
                m_camera.m_orientation.z = glm::atan(camera_forward.x, camera_forward.z);
            }
        }

        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), m_camera.m_orientation.z, gfx_math::up.xyz()) * glm::rotate(glm::mat4(1.0f), m_camera.m_orientation.y, gfx_math::right.xyz());
        glm::mat4 translation = glm::translate(glm::mat4(1.0f), m_camera.m_focus_point);
        // If we are in perspective mode, we need to translate the camera back by the zoom level to maintain the correct distance from the focus point.
        // Otherwise, in orthographic mode, we translate the camera back by half the far plane distance to ensure the entire scene is visible.
        glm::mat4 center_offset = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, m_camera.m_projection == Camera::Projection::Perspective ? m_camera.m_zoom_level : Camera::c_far_plane / 2.0f));
        m_camera.m_view_matrix = glm::inverse(translation * rotation * center_offset);
    }
}