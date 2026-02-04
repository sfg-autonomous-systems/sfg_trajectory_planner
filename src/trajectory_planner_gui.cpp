#include "sfg_trajectory_planner/trajectory_planner_gui.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <magic_enum.hpp>

// Specialization for ImGuizmo::OPERATION enum to increase the range of values considered by magic_enum.
template <>
struct magic_enum::customize::enum_range<ImGuizmo::OPERATION>
{
    static constexpr int min = 0;
    static constexpr int max = ImGuizmo::UNIVERSAL;
};

namespace sfg_trajectory_planner
{
    static constexpr glm::vec3 right() { return glm::vec3(1.0f, 0.0f, 0.0f); }
    static constexpr glm::vec3 up() { return glm::vec3(0.0f, 1.0f, 0.0f); }
    static constexpr glm::vec3 forward() { return glm::vec3(0.0f, 0.0f, 1.0f); }

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

    TrajectoryPlannerGui::TrajectoryPlannerGui(rclcpp::Node *node)
        : GuiElement(),
          m_node(node)
    {
        // Declare and retrieve ROS parameters.
        m_config.m_grid_origin = glm::vec3(
            m_node->declare_parameter("gui.grid.origin.x", m_config.m_grid_origin.x),
            m_node->declare_parameter("gui.grid.origin.y", m_config.m_grid_origin.y),
            m_node->declare_parameter("gui.grid.origin.z", m_config.m_grid_origin.z));
        m_config.m_grid_orientation = glm::vec3(
            m_node->declare_parameter("gui.grid.orientation.roll", m_config.m_grid_orientation.x),
            m_node->declare_parameter("gui.grid.orientation.pitch", m_config.m_grid_orientation.y),
            m_node->declare_parameter("gui.grid.orientation.yaw", m_config.m_grid_orientation.z));
        m_config.m_grid_scale = glm::vec3(
            m_node->declare_parameter("gui.grid.scale.x", m_config.m_grid_scale.x),
            m_node->declare_parameter("gui.grid.scale.y", m_config.m_grid_scale.y),
            m_node->declare_parameter("gui.grid.scale.z", m_config.m_grid_scale.z));
        m_config.m_grid_size = m_node->declare_parameter("gui.grid.size", m_config.m_grid_size);

        m_camera.m_orbit_speed = m_node->declare_parameter("gui.camera.orbit_speed", m_camera.m_orbit_speed);
        m_camera.m_zoom_speed = m_node->declare_parameter("gui.camera.zoom_speed", m_camera.m_zoom_speed);
        m_camera.m_pan_speed = m_node->declare_parameter("gui.camera.pan_speed", m_camera.m_pan_speed);

        m_object_matrix = glm::mat4(1.0f);
        m_grid_matrix = glm::translate(glm::mat4(1.0f), m_config.m_grid_origin);
        m_grid_matrix *= (glm::rotate(glm::mat4(1.0f), m_config.m_grid_orientation.x, forward()) *
                          glm::rotate(glm::mat4(1.0f), m_config.m_grid_orientation.y, right()) *
                          glm::rotate(glm::mat4(1.0f), m_config.m_grid_orientation.z, up()));
        m_grid_matrix = glm::scale(m_grid_matrix, m_config.m_grid_scale);
    }

    TrajectoryPlannerGui::~TrajectoryPlannerGui() {}

    void TrajectoryPlannerGui::render_internal()
    {
        auto &io = ImGui::GetIO();

        ImGuizmo::BeginFrame();
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin(
            "Main Window",
            nullptr,
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar);

        if (ImGui::BeginTable("TopColumns", 2, ImGuiTableFlags_Resizable))
        {
            ImGui::TableSetupColumn("Viewport", ImGuiTableColumnFlags_WidthStretch, 2.0f);
            ImGui::TableSetupColumn("Inspector", ImGuiTableColumnFlags_WidthStretch, 1.0f);
            ImGui::TableNextColumn();
            render_viewport();
            ImGui::TableNextColumn();
            render_inspector();
        }
        ImGui::EndTable();
        ImGui::End();
    }

    void TrajectoryPlannerGui::render_viewport()
    {
        ImGui::BeginChild("Viewport");
        ImGui::Text("Viewport");
        ImGui::SameLine();
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

        ImGuizmo::SetRect(ImGui::GetCursorPosX(), ImGui::GetCursorPosY(), ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);
        ImGuizmo::SetDrawlist();
        update_camera_matrices();
        ImGuizmo::DrawGrid(glm::value_ptr(m_camera.m_view_matrix), glm::value_ptr(m_camera.m_projection_matrix), glm::value_ptr(m_grid_matrix), m_config.m_grid_size);
        ImGuizmo::Manipulate(
            glm::value_ptr(m_camera.m_view_matrix),
            glm::value_ptr(m_camera.m_projection_matrix),
            m_gizmo_operation,
            m_gizmo_mode,
            glm::value_ptr(m_object_matrix));

        // ImGuizmo::ViewManipulate enables rotating the view by holding down the left mouse button and then moving the mouse.
        // But in our application we use the right mouse button for orbiting the camera, so we temporarily disable the left mouse button input.
        ImGuiIO &io = ImGui::GetIO();
        auto old_mouse_delta = io.MouseDelta;
        io.MouseDelta = ImVec2(0, 0);
        ImGuizmo::ViewManipulate(
            glm::value_ptr(m_camera.m_view_matrix),
            Config::c_view_gizmo_distance,
            ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY()),
            ImVec2(Config::c_view_gizmo_size, Config::c_view_gizmo_size),
            0);
        io.MouseDelta = old_mouse_delta;

        ImGui::EndChild();
    }

    void TrajectoryPlannerGui::render_inspector()
    {
        ImGui::BeginChild("Inspector");
        ImGui::Text("Inspector");
        ImGui::EndChild();
    }

    void TrajectoryPlannerGui::update_camera_matrices()
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
            glm::vec3 camera_forward = glm::normalize(view_inverse * glm::vec4(forward(), 0.0f));
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

            // Implement panning. If the middle mouse button is held, adjust the camera focus point based on mouse movement.
            if (ImGui::IsMouseDown(ImGuiMouseButton_Middle))
            {
                auto pan_speed = m_camera.m_pan_speed * m_camera.m_zoom_level;
                glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), m_camera.m_orientation.z, up()) * glm::rotate(glm::mat4(1.0f), m_camera.m_orientation.y, right());
                m_camera.m_focus_point -= io.MouseDelta.x * pan_speed * (rotation * glm::vec4(right(), 0.0f));
                m_camera.m_focus_point += io.MouseDelta.y * pan_speed * (rotation * glm::vec4(up(), 0.0f));
            }
        }

        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), m_camera.m_orientation.z, up()) * glm::rotate(glm::mat4(1.0f), m_camera.m_orientation.y, right());
        glm::mat4 translation = glm::translate(glm::mat4(1.0f), m_camera.m_focus_point);
        // If we are in perspective mode, we need to translate the camera back by the zoom level to maintain the correct distance from the focus point.
        // Otherwise, in orthographic mode, we translate the camera back by half the far plane distance to ensure the entire scene is visible.
        glm::mat4 center_offset = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, m_camera.m_projection == Camera::Projection::Perspective ? m_camera.m_zoom_level : Camera::c_far_plane / 2.0f));
        m_camera.m_view_matrix = glm::inverse(translation * rotation * center_offset);
    }
}