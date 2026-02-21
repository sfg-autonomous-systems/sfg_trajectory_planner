#include "sfg_trajectory_planner/app/core/waypoint.hpp"

#include <glm/glm.hpp>

#include "sfg_trajectory_planner/app/core/trajectory.hpp"
#include "sfg_trajectory_planner/engine/core/serialization/abstract_serializer.hpp"

namespace sfg_trajectory_planner::app::core
{

    Waypoint::Waypoint(engine::core::Transform transform_ls, float time_from_last, Constraints constraints)
        : m_transform_ls(transform_ls),
          m_time_from_last(time_from_last),
          m_constraints(constraints)
    {
    }

    void Waypoint::serialize(engine::core::serialization::AbstractSerializer *serializer) const
    {
        m_transform_ls.serialize(serializer);
        serializer->serialize("time_from_last", m_time_from_last);
        serializer->serialize("constraints", static_cast<size_t>(m_constraints));
    }

    void Waypoint::deserialize(engine::core::serialization::AbstractSerializer *serializer)
    {
        m_transform_ls.deserialize(serializer);
        m_time_from_last = serializer->deserialize<float>("time_from_last");
        m_constraints = static_cast<Waypoint::Constraints>(serializer->deserialize<size_t>("constraints"));
    }
}