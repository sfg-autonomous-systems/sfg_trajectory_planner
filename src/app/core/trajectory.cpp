#include "sfg_trajectory_planner/app/core/trajectory.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include "sfg_imgui_vendor/push_id_guard.hpp"
#include "sfg_trajectory_planner/engine/core/gfx/renderer.hpp"
#include "sfg_trajectory_planner/engine/core/gfx/utils.hpp"
#include "sfg_trajectory_planner/engine/core/serialization/abstract_serializer.hpp"
#include "sfg_trajectory_planner/engine/core/scene.hpp"
#include "sfg_trajectory_planner/engine/editor/selection_context.hpp"

namespace sfg_trajectory_planner::app::core
{
    Trajectory::Trajectory(engine::core::SceneObject::ConstructionKey key, engine::core::Scene &scene, uuids::uuid uuid) : SceneObject(key, scene, uuid) {}

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
            Waypoint waypoint;
            serializer->next_item();
            waypoint.deserialize(serializer);
            m_waypoints.push_back(waypoint);
        }
        serializer->end_sequence();
    }

    void Trajectory::render_object(engine::core::gfx::Renderer &renderer)
    {
        auto object_to_world_matrix = get_object_to_world_matrix();

        for (size_t index = 1; index < m_waypoints.size(); index++)
        {
            glm::vec3 start = glm::vec3(object_to_world_matrix * glm::vec4(m_waypoints[index - 1].m_transform.get_translation(), 1.0f));
            glm::vec3 end = glm::vec3(object_to_world_matrix * glm::vec4(m_waypoints[index].m_transform.get_translation(), 1.0f));
            renderer.add_line(start, end, m_color);
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

    std::vector<Waypoint> &Trajectory::get_waypoints()
    {
        return m_waypoints;
    }

    void Trajectory::set_topic_name(const std::string &topic_name)
    {
        // ToDo: Validate topic name.
        m_topic_name = topic_name;
    }

    void Trajectory::set_frame_id(const std::string &frame_id)
    {
        // ToDo: Validate frame ID.
        m_frame_id = frame_id;
    }

    void Trajectory::set_time_from_start(float time_from_start)
    {
        m_time_from_start = glm::max(time_from_start, 0.0f);
    }

    void Trajectory::set_color(const glm::vec3 &color)
    {
        m_color = color;
    }
}