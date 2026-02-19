#pragma once

#include <magic_enum_flags.hpp>

#include "sfg_trajectory_planner/engine/core/transform.hpp"

namespace sfg_trajectory_planner::app::core
{
    class Waypoint : public engine::core::serialization::ISerializable
    {
    public:
        friend class Trajectory;

        enum class Constraints : std::uint64_t
        {
            None = 0,
            AlignWithPrevious = 1 << 0,
            AlignWithNext = 1 << 1
        };

        void serialize(engine::core::serialization::AbstractSerializer *serializer) const override;
        void deserialize(engine::core::serialization::AbstractSerializer *serializer) override;

    private:
        Waypoint() = default;
        Waypoint(engine::core::Transform transform, float time_from_last, Constraints constraints);

        engine::core::Transform m_transform;
        float m_time_from_last = 1.0f;
        Constraints m_constraints = Constraints::None;
    };
}

template <>
struct magic_enum::customize::enum_range<sfg_trajectory_planner::app::core::Waypoint::Constraints>
{
    static constexpr auto is_flags = true;
};