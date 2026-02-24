#include "sfg_trajectory_planner/app/core/trajectory.hpp"

#include <fmt/format.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include "sfg_imgui_vendor/push_id_guard.hpp"
#include "sfg_trajectory_planner/engine/core/gfx/camera.hpp"
#include "sfg_trajectory_planner/engine/core/gfx/renderer.hpp"
#include "sfg_trajectory_planner/engine/core/gfx/utils.hpp"
#include "sfg_trajectory_planner/engine/core/scene.hpp"
#include "sfg_trajectory_planner/engine/core/serialization/abstract_serializer.hpp"
#include "sfg_trajectory_planner/engine/editor/selection_context.hpp"

namespace sfg_trajectory_planner::app::core
{
    Trajectory::Trajectory(engine::core::SceneObject::ConstructionKey key, engine::core::Scene &scene, uuids::uuid uuid) : SceneObject(key, scene, uuid)
    {
    }

    void Trajectory::serialize(engine::core::serialization::AbstractSerializer *serializer) const
    {
        SceneObject::serialize(serializer);

        serializer->serialize("topic_name", m_topic_name);
        serializer->serialize("frame_id", m_frame_id);
        serializer->serialize("time_from_start", m_time_from_start);
        serializer->serialize("color", std::vector<float>{m_color.r, m_color.g, m_color.b});
        serializer->begin_sequence("waypoints", engine::core::serialization::AbstractSerializer::Mode::Write);

        for (const auto &waypoint : m_waypoints)
        {
            serializer->next_item();
            waypoint.serialize(serializer);
        }
        serializer->end_sequence();
    }

    void Trajectory::deserialize(engine::core::serialization::AbstractSerializer *serializer)
    {
        SceneObject::deserialize(serializer);

        m_topic_name = serializer->deserialize<std::string>("topic_name");
        m_frame_id = serializer->deserialize<std::string>("frame_id");
        m_time_from_start = serializer->deserialize<float>("time_from_start");
        auto color = serializer->deserialize<std::vector<float>>("color");
        m_color = glm::vec3(color[0], color[1], color[2]);

        auto waypoint_count = serializer->begin_sequence("waypoints", engine::core::serialization::AbstractSerializer::Mode::Read);
        m_waypoints.clear();
        m_waypoints.reserve(waypoint_count);

        for (size_t index = 0; index < waypoint_count; index++)
        {
            serializer->next_item();
            m_waypoints.emplace_back(Waypoint());
            m_waypoints.back().deserialize(serializer);
        }
        serializer->end_sequence();
    }

    void Trajectory::render_object(engine::core::gfx::Renderer &renderer)
    {
        auto ls_to_ws_matrix = get_ls_to_ws_matrix();

        for (size_t index = 1; index < m_waypoints.size(); index++)
        {
            renderer.add_line(
                glm::vec3(ls_to_ws_matrix * glm::vec4(m_waypoints[index - 1].m_transform_ls.get_translation(), 1.0f)),
                glm::vec3(ls_to_ws_matrix * glm::vec4(m_waypoints[index].m_transform_ls.get_translation(), 1.0f)),
                m_color);
        }

        auto time_from_start = m_time_from_start;

        for (size_t index = 0; index < m_waypoints.size(); index++)
        {
            time_from_start += m_waypoints[index].m_time_from_last;
            auto color = ImGui::GetStyleColorVec4(ImGuiCol_Text);

            renderer.add_text(
                ls_to_ws_matrix,
                m_waypoints[index].m_transform_ls.get_translation(),
                "t+" + fmt::format("{:.1f}", time_from_start),
                2.0f * ImGui::GetFontSize(),
                glm::vec3(color.x, color.y, color.z),
                engine::core::gfx::TextAnchor::Center);
        }
    }

    const std::string &Trajectory::get_topic_name() const
    {
        return m_topic_name;
    }

    const std::string &Trajectory::get_frame_id() const
    {
        return m_frame_id;
    }

    float Trajectory::get_time_from_start() const
    {
        return m_time_from_start;
    }

    glm::vec3 Trajectory::get_color() const
    {
        return m_color;
    }

    size_t Trajectory::get_waypoint_count() const
    {
        return m_waypoints.size();
    }

    void Trajectory::set_topic_name(std::string topic_name)
    {
        // ToDo: Validate topic name.
        m_topic_name = std::move(topic_name);
    }

    void Trajectory::set_frame_id(std::string frame_id)
    {
        // ToDo: Validate frame ID.
        m_frame_id = std::move(frame_id);
    }

    void Trajectory::set_time_from_start(float time_from_start)
    {
        m_time_from_start = glm::max(time_from_start, 0.0f);
    }

    void Trajectory::set_color(glm::vec3 color)
    {
        m_color = std::move(color);
    }

    bool Trajectory::empty() const
    {
        return m_waypoints.empty();
    }

    void Trajectory::add_waypoint()
    {
        // We'll place new waypoint's one unit forward from the last waypoint.
        auto last_transform_ls = m_waypoints.size() == 0 ? glm::mat4(1.0f) : m_waypoints.back().m_transform_ls;
        // We'll also inherit the constraints of the last waypoint.
        auto last_constraints = m_waypoints.size() == 0 ? core::Waypoint::Constraints::None : m_waypoints.back().m_constraints;

        add_waypoint(last_transform_ls.translate(engine::core::gfx::utils::s_forward.xyz()), 1.0f, last_constraints);
    }

    void Trajectory::remove_waypoint(size_t index)
    {
        if (index < m_waypoints.size())
        {
            m_waypoints.erase(m_waypoints.begin() + index);
        }
    }

    const engine::core::Transform &Trajectory::get_waypoint_transform_ls(size_t index) const
    {
        return m_waypoints[index].m_transform_ls;
    }

    float Trajectory::get_waypoint_time_from_last(size_t index) const
    {
        return m_waypoints[index].m_time_from_last;
    }

    Waypoint::Constraints Trajectory::get_waypoint_constraints(size_t index) const
    {
        return m_waypoints[index].m_constraints;
    }

    void Trajectory::set_waypoint_transform_ls(size_t index, engine::core::Transform transform_ls)
    {
        if (!can_translate_waypoint(index))
        {
            transform_ls.set_translation(m_waypoints[index].m_transform_ls.get_translation());
        }

        if (!can_rotate_waypoint(index))
        {
            transform_ls.set_rotation(m_waypoints[index].m_transform_ls.get_rotation());
        }

        if (!can_scale_waypoint(index))
        {
            transform_ls.set_scale(m_waypoints[index].m_transform_ls.get_scale());
        }
        m_waypoints[index].m_transform_ls = std::move(transform_ls);

        enforce_waypoint_constraints(index - 1);
        enforce_waypoint_constraints(index);
        enforce_waypoint_constraints(index + 1);
    }

    void Trajectory::set_waypoint_time_from_last(size_t index, float time_from_last)
    {
        m_waypoints[index].m_time_from_last = glm::max(time_from_last, 0.0f);
    }

    void Trajectory::set_waypoint_constraints(size_t index, Waypoint::Constraints constraints)
    {
        auto &old_constraints = m_waypoints[index].m_constraints;

        if (constraints == old_constraints)
        {
            return;
        }

        using namespace magic_enum::bitwise_operators;

        if (index == 0)
        {
            constraints &= ~Waypoint::Constraints::AlignWithPrevious;
        }
        else if (index == m_waypoints.size() - 1)
        {
            constraints &= ~Waypoint::Constraints::AlignWithNext;
        }

        if ((constraints & Waypoint::Constraints::AlignWithPrevious) != Waypoint::Constraints::None && (constraints & Waypoint::Constraints::AlignWithNext) != Waypoint::Constraints::None)
        {
            if ((old_constraints & Waypoint::Constraints::AlignWithPrevious) != Waypoint::Constraints::None)
            {
                constraints &= ~Waypoint::Constraints::AlignWithPrevious;
            }
            else
            {
                constraints &= ~Waypoint::Constraints::AlignWithNext;
            }
        }
        old_constraints = constraints;
        enforce_waypoint_constraints(index);
    }

    bool Trajectory::can_translate_waypoint(size_t) const
    {
        return true;
    }

    bool Trajectory::can_rotate_waypoint(size_t index) const
    {
        using namespace magic_enum::bitwise_operators;

        if ((m_waypoints[index].m_constraints & Waypoint::Constraints::AlignWithPrevious) != Waypoint::Constraints::None)
        {
            return false;
        }

        if ((m_waypoints[index].m_constraints & Waypoint::Constraints::AlignWithNext) != Waypoint::Constraints::None)
        {
            return false;
        }
        return true;
    }

    bool Trajectory::can_scale_waypoint(size_t) const
    {
        return false;
    }

    void Trajectory::enforce_waypoint_constraints(size_t index)
    {
        if (index >= m_waypoints.size() || m_waypoints.size() < 2)
        {
            return;
        }

        using namespace magic_enum::bitwise_operators;

        if ((m_waypoints[index].m_constraints & Waypoint::Constraints::AlignWithPrevious) != Waypoint::Constraints::None)
        {
            glm::vec3 direction_ls = m_waypoints[index].m_transform_ls.get_translation() - m_waypoints[index - 1].m_transform_ls.get_translation();
            m_waypoints[index].m_transform_ls.look_in(direction_ls);
        }

        if ((m_waypoints[index].m_constraints & Waypoint::Constraints::AlignWithNext) != Waypoint::Constraints::None)
        {
            glm::vec3 direction_ls = m_waypoints[index + 1].m_transform_ls.get_translation() - m_waypoints[index].m_transform_ls.get_translation();
            m_waypoints[index].m_transform_ls.look_in(direction_ls);
        }
    }
}