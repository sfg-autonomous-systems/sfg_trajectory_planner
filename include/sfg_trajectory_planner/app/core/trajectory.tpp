#include "sfg_trajectory_planner/app/core/trajectory.hpp"

namespace sfg_trajectory_planner::app::core
{
    template <typename... Args>
    void Trajectory::add_waypoint(Args &&...args)
    {
        m_waypoints.emplace_back(Waypoint(std::forward<Args>(args)...));
        enforce_waypoint_constraints(m_waypoints.size() - 2);
        enforce_waypoint_constraints(m_waypoints.size() - 1);
    }
}