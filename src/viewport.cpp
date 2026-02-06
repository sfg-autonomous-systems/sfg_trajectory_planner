#include "sfg_trajectory_planner/viewport.hpp"

#include "sfg_trajectory_planner/gfx_math.hpp"

#include <magic_enum.hpp>
#include <glm/gtc/type_ptr.hpp>

// Specialization for ImGuizmo::OPERATION enum to increase the range of values considered by magic_enum.
template <>
struct magic_enum::customize::enum_range<ImGuizmo::OPERATION>
{
    static constexpr int min = 0;
    static constexpr int max = ImGuizmo::UNIVERSAL;
};

namespace sfg_trajectory_planner
{
    template <typename EnumType>
    bool render_enum_cycle_button(const std::string &prefix, EnumType &value, std::initializer_list<EnumType> whitelist = {})
    {
        auto values = whitelist.size() > 0 ? std::vector(whitelist) : std::vector(magic_enum::enum_values<EnumType>().begin(), magic_enum::enum_values<EnumType>().end());

        if (ImGui::Button((prefix + std::string(magic_enum::enum_name(value))).data()))
        {
            auto iterator = std::find(values.begin(), values.end(), value);
            value = iterator == values.end() ? values[0] : values[(std::distance(values.begin(), iterator) + 1) % values.size()];
            return true;
        }
        return false;
    }

    Viewport::Viewport(rclcpp::Node *node) : GuiElement()
    {
        // Declare and retrieve ROS parameters.
        m_config.m_grid_origin = glm::vec3(
            node->declare_parameter("gui.viewport.grid.origin.x", m_config.m_grid_origin.x),
            node->declare_parameter("gui.viewport.grid.origin.y", m_config.m_grid_origin.y),
            node->declare_parameter("gui.viewport.grid.origin.z", m_config.m_grid_origin.z));
        m_config.m_grid_scale = glm::vec3(
            node->declare_parameter("gui.viewport.grid.scale.x", m_config.m_grid_scale.x),
            node->declare_parameter("gui.viewport.grid.scale.y", m_config.m_grid_scale.y),
            node->declare_parameter("gui.viewport.grid.scale.z", m_config.m_grid_scale.z));
        m_config.m_grid_size = node->declare_parameter("gui.viewport.grid.size", m_config.m_grid_size);
        m_camera.m_orbit_speed = node->declare_parameter("gui.viewport.camera.orbit_speed", m_camera.m_orbit_speed);
        m_camera.m_zoom_speed = node->declare_parameter("gui.viewport.camera.zoom_speed", m_camera.m_zoom_speed);

        m_grid_matrix = glm::translate(glm::mat4(1.0f), m_config.m_grid_origin);
        m_grid_matrix = glm::scale(m_grid_matrix, m_config.m_grid_scale);
    }

    void Viewport::render_internal()
    {
        ImGui::BeginChild("Viewport", ImVec2(0, 0), ImGuiChildFlags_Border);
        render_enum_cycle_button("Space: ", m_gizmo_mode);
        ImGui::SameLine();
        render_enum_cycle_button("Mode: ", m_gizmo_operation, {ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::OPERATION::ROTATE, ImGuizmo::OPERATION::SCALE});
        ImGui::SameLine();

        if (render_enum_cycle_button("View: ", m_camera.m_projection))
        {
            // When switching between projections we should adjust the zoom level to maintain a consistent view.
            // The zoom level in perspective mode represents the distance from the focus point.
            // The zoom level in orothographic mode represents half of the vertical viewport.
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
            ImVec2(m_camera.m_viewport.x, m_camera.m_viewport.y),
            ImVec2(Config::c_view_gizmo_size, Config::c_view_gizmo_size),
            0);
        io.MouseDelta = old_mouse_delta;
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
        auto &io = ImGui::GetIO();
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

        if (ImGui::IsWindowHovered() && !ImGuizmo::IsUsingViewManipulate() && !ImGuizmo::IsUsing())
        {
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
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), m_camera.m_orientation.z, gfx_math::up.xyz()) * glm::rotate(glm::mat4(1.0f), m_camera.m_orientation.y, gfx_math::right.xyz());
        glm::mat4 translation = glm::translate(glm::mat4(1.0f), m_camera.m_focus_point);
        // If we are in perspective mode, we need to translate the camera back by the zoom level to maintain the correct distance from the focus point.
        // Otherwise, in orthographic mode, we translate the camera back by half the far plane distance to ensure the entire scene is visible.
        glm::mat4 center_offset = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, m_camera.m_projection == Camera::Projection::Perspective ? m_camera.m_zoom_level : Camera::c_far_plane / 2.0f));
        m_camera.m_view_matrix = glm::inverse(translation * rotation * center_offset);
    }
}